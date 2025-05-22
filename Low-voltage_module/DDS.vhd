library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.MATH_REAL.ALL;


entity DDS is
    port(
        -- i_clk            - system clock
        -- i_rst            - system reset
        -- i_begin          - goes to logic 1 for 1 clock cycle as signal to start next calculation
        -- i_FTW            - frequency tuning word (X"FFFFFFFF" = 360°)
        -- i_amplitude      - amplitude of sine/cosine signal
        -- o_ready          - ready flag for main code, (0 if not ready, 1 if new value of sin and cos was calculated)
        -- o_code           - 20 bit result code for DAC11001B
        
        i_clk               : in    std_logic;
        i_rst               : in    std_logic;
        i_begin             : in    std_logic;
        i_FTW               : in	unsigned(31 downto 0);
        i_amplitude         : in    unsigned(31 downto 0);
        
        o_ready             : out   std_logic;
        o_code              : out   std_logic_vector(19 downto 0)
        );
end DDS;

architecture Behavioral of DDS is

    component CORDIC
        port(
            i_clk               : in    std_logic;
            i_rst               : in    std_logic;
            i_begin             : in    std_logic;
            i_amplitude         : in    unsigned(31 downto 0);
            i_angle             : in    unsigned(31 downto 0);
            o_done              : out   std_logic;
            o_sin               : out   unsigned(63 downto 0);
            o_cos               : out   unsigned(63 downto 0)
        );
    end component;
    
    -- C_DAC_OFFSET             - half of DAC11001B range, with this value set, DAC output is zero volts
    
    constant    C_DAC_OFFSET        : unsigned(19 downto 0) := X"7FFFF";
    
    -- r_DDS_state              - state of DDS
    -- r_phase                  - angle to be send to CORDIC module, range 0° -> +90° (X"00000000" -> X"40000000")
    -- r_phase_acc              - phase accumulator, after each calculation, i_FTW is added to r_phase_acc
    -- r_CORDIC_sin             - result of sine calculation from CORDIC module 
    -- r_CORDIC_cos             - result of cosine calculation from CORDIC module 
    -- r_CORDIC_begin           - signal for CORDIC module to start calculation
    -- r_CORDIC_done            - signal from CORDIC module that calculation is finished (0 if not done, 1 if done)
    
    type	    t_DDS_state is (t_IDLE, t_WAITING_FOR_CORDIC);
    
    signal	    r_DDS_state		    : t_DDS_state		        := t_IDLE;
    signal	    r_phase			    : unsigned(31 downto 0)     := (others => '0');
    signal	    r_phase_acc		    : unsigned(31 downto 0)     := (others => '0');
    signal	    r_CORDIC_sin	    : unsigned(63 downto 0)     := (others => '0');
    signal	    r_CORDIC_cos	    : unsigned(63 downto 0)     := (others => '0');
    signal      r_CORDIC_begin      : std_logic                 := '0';
    signal      r_CORDIC_done       : std_logic                 := '0';
    
begin
    
    -- process p_DDS waits for i_begin signal from main code, calculation of 20-bit long code for DAC11001B starts
    -- DDS is adding i_FTW to r_phase_acc, so module calculates DAC code for whole range from 0° to 360°
    -- CORDIC module can calculate angles in range 0° -> +90°, so DDS adjust every angle to this range
    -- after result is received from CORDIC module, DDS adjust code by adding or subtracting DAC offset, thus setting zero of sine signal
    p_DDS : process (i_clk)
    begin
    
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_DDS_state <= t_IDLE;
                r_phase <= (others => '0');
                r_phase_acc <= (others => '0');
                o_ready <= '1';
                o_code <= (others => '0');
                r_CORDIC_begin <= '0';
            else
                case r_DDS_state is
                    
                    -- =========================================
                    -- wait for i_begin signal from main program
                    when t_IDLE =>
                        
                        if (i_begin = '1') then
                            r_DDS_state <= t_WAITING_FOR_CORDIC;
                            o_ready <= '0';
                            r_CORDIC_begin <= '1';
                            -- adjust every angle to range 0° -> +90°
                            if (r_phase_acc <= X"40000000") then        -- 0° -> +90°
                                r_phase <= r_phase_acc;
                            elsif (r_phase_acc <= X"80000000") then     -- +90° -> +180°
                                r_phase <= X"80000000" - r_phase_acc;
                            elsif (r_phase_acc <= X"C0000000") then     -- +180° -> +270°
                                r_phase <= r_phase_acc - X"80000000";
                            else                                        -- +270° -> +360°
                                r_phase <= X"FFFFFFFF" - r_phase_acc; 
                            end if;
                        else
                            o_ready <= '1';
                        end if;
                    
                    -- ===================================================
                    -- wait until CORDIC calculates sin() and cos() values
                    when t_WAITING_FOR_CORDIC =>
                        r_CORDIC_begin <= '0';
                        
                        if ((r_CORDIC_done = '1') and (r_CORDIC_begin /= '1')) then
                            r_DDS_state <= t_IDLE;
                            o_ready <= '1';
                            -- add or subract DAC offset to bias signal around zero
                            if (r_phase_acc <= X"40000000") then                                            -- 0° -> +90°
                                o_code <= std_logic_vector(r_CORDIC_sin(49 downto 30) + C_DAC_OFFSET);
                            elsif (r_phase_acc <= X"80000000") then                                         -- +90° -> +180°
                                o_code <= std_logic_vector(r_CORDIC_sin(49 downto 30) + C_DAC_OFFSET);
                            elsif (r_phase_acc <= X"C0000000") then                                         -- +180° -> +270°
                                o_code <= std_logic_vector(C_DAC_OFFSET - r_CORDIC_sin(49 downto 30) + 1);
                            else                                                                            -- +270° -> +360°
                                o_code <= std_logic_vector(C_DAC_OFFSET - r_CORDIC_sin(49 downto 30) + 1);
                            end if;
                            
                            r_phase_acc <= r_phase_acc + i_FTW;         -- increase angle by frequency tuning word
                        end if;
                        
                    when others =>
                
                end case;
            end if;	
        end if;
    end process;
    
    -- instance of CORDIC
    instance_CORDIC : CORDIC
        port map(
            i_clk => i_clk,
            i_rst => i_rst,
            i_begin => r_CORDIC_begin,
            i_amplitude => i_amplitude,
            i_angle => r_phase,
            o_done => r_CORDIC_done,
            o_sin => r_CORDIC_sin,
            o_cos => r_CORDIC_cos
            );
            
end Behavioral;
