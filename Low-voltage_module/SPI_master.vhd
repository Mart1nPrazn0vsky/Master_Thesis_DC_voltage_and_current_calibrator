library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.MATH_REAL.ALL;


entity SPI_master is
    generic(
        -- G_CLOCK_FREQ     - FPGA clock frequency
        -- G_SCLK_FREQ      - SPI SCLK frequency
        
        G_CLOCK_FREQ        : real      := 12.0e6;
        G_SCLK_FREQ         : positive  := 12000000
        );
    port(
        -- i_clk            - system clock
        -- i_rst            - system reset
        -- i_begin          - goes to logic 1 for 1 clock period when transmission of data is supposed to start
        -- i_data           - 32 bits of data to be send by master
        -- i_MISO           - MISO pin of SPI bus
        -- o_MOSI           - MOSI pin of SPI bus
        -- o_CS             - CS pin of SPI bus
        -- o_SCLK           - SLCK pin of SPI bus
        -- o_SPI_valid      - goes to logic 1 for 1 clock cycle when byte of data is received
        -- o_SPI_busy       - busy flag (0 = not busy, 1 = busy)
        -- o_data           - 32 bits of data read from slave
        
        i_clk               : in    std_logic;
        i_rst               : in    std_logic;
        
        i_begin             : in    std_logic;
        i_data              : in    std_logic_vector(31 downto 0);
        i_MISO              : in    std_logic;
        
        o_MOSI              : out   std_logic;
        o_CS                : out   std_logic;
        o_SCLK              : out   std_logic;
        o_SPI_valid         : out   std_logic;
        o_SPI_busy          : out   std_logic;
        o_data              : out   std_logic_vector(31 downto 0)
        );
end SPI_master;

architecture Behavioral of SPI_master is

    -- C_DELAY_CYCLES               - number of clock cycles between 2 edges in FPGA
    -- C_TCSHIGH                    - number of clock cycles when CS must be logic 1 between 2 SPI transmission cycles
    -- C_TCSS                       - number of clock cycles between CS falling edge and first SLCK falling edge (start of transmission)
    -- C_TCSH                       - number of clock cycles between last SLCK falling edge and CS rising edge (end of transmission)
    
    constant    C_DELAY_CYCLES      : natural                           := natural(ceil(G_CLOCK_FREQ / real(G_SCLK_FREQ))) - 1;
    constant    C_TCSHIGH           : positive                          := positive(ceil(50.0e-9 * G_CLOCK_FREQ));
    constant    C_TCSS              : positive                          := positive(ceil(18.0e-9 * G_CLOCK_FREQ));
    constant    C_TCSH              : positive                          := positive(ceil(10.0e-9 * G_CLOCK_FREQ));
    
    -- r_state                      - state of SPI master
    -- r_shift_register             - shift register for sending data, MSB is connected to MOSI
    -- r_MISO_data                  - shift register for received data from MISO
    -- r_counter                    - counter for counting clock cycles
    -- r_edge_counter               - counter for counting SLCK falling edges
    
    type        t_SPI_state is (t_IDLE, t_RISING_EDGE, t_FALLING_EDGE, t_END, t_END_WAIT);
    
    signal      r_SPI_state         : t_SPI_state                           := t_IDLE;
    signal      r_shift_register    : std_logic_vector(31 downto 0)         := (others => '0');
    signal      r_MISO_data         : std_logic_vector(31 downto 0)         := (others => '0');
    signal      r_counter           : integer range 0 to C_DELAY_CYCLES + 1 := 0;
    signal      r_edge_counter      : integer range 0 to 31                 := 0;

begin
    
    -- process p_SPI_master is used for serial communication with DAC11001B
    -- 32-bit data packets, SPI mode 1 (CPOL = 0, CPHA = 1, data sampled on falling edge, shifted out on rising edge)
    p_SPI_master : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_counter <= C_TCSS;
                r_edge_counter <= 0;
                r_SPI_state <= t_IDLE;
                o_CS <= '1';
                o_SCLK <= '0';
                o_SPI_busy <= '0';
                r_shift_register <= (others => '0');
                r_MISO_data <= (others => '0');
                o_SPI_valid <= '0';
            else
                case r_SPI_state is
                    -- ================================================================================
                    -- wait until i_begin pulse, then set first rising edge and shift first bit of data
                    when t_IDLE =>
                        r_counter <= C_TCSS;
                        r_edge_counter <= 0;
                        o_SPI_valid <= '0';
                        if (i_begin = '1') then
                            r_shift_register <= i_data;
                            r_SPI_state <= t_FALLING_EDGE;
                            o_SPI_busy <= '1';              -- set busy flag
                            o_SCLK <= '1';                  -- first rising edge
                            o_CS <= '0';
                        else
                            r_shift_register <= (others => '0');
                            o_SPI_busy <= '0';
                            o_SCLK <= '0';
                            o_CS <= '1';
                        end if;
                    
                    -- ======================================================
                    -- at rising edge, new bit of data is shifted out by MOSI
                    when t_RISING_EDGE =>
                        if (r_counter = 0) then
                            o_SCLK <= '1';                  -- set rising edge
                            r_counter <= C_DELAY_CYCLES;
                            r_SPI_state <= t_FALLING_EDGE;
                            r_shift_register(31 downto 0) <= r_shift_register(30 downto 0) & "0";   -- shift new bit to MOSI
                        else
                            r_counter <= r_counter - 1;     -- decrease time counter
                        end if;
                    
                    -- ============================================================
                    -- at falling edge, new bit from DAC11001B is sampled from MISO
                    when t_FALLING_EDGE =>
                        if (r_counter = 0) then
                            o_SCLK <= '0';                  -- set falling edge
                            r_MISO_data(31 downto 0) <= r_MISO_data(30 downto 0) & i_MISO;      -- sample new bit from MISO
                            r_SPI_state <= t_RISING_EDGE;
                            if (r_edge_counter = 31) then
                                r_SPI_state <= t_END;
                                r_counter <= C_TCSH;
                            else
                                r_edge_counter <= r_edge_counter + 1;
                                r_counter <= C_DELAY_CYCLES;
                            end if;
                        else
                            r_counter <= r_counter - 1;     -- decrease time counter
                        end if;
                    
                    -- ===================================================================================
                    -- after 32 bits were transmitted, SPI waits for time specified by DAC11001B datasheet
                    when t_END =>
                        if (r_counter = 0) then
                            r_SPI_state <= t_END_WAIT;
                            o_CS <= '1';
                            r_counter <= C_TCSHIGH;
                        else
                            r_counter <= r_counter - 1;     -- decrease time counter
                        end if;
                    
                    -- ==================================================================
                    -- DAC11001B datasheet specifies minimum time between 2 transmissions
                    when t_END_WAIT =>
                        if (r_counter = 0) then
                            r_SPI_state <= t_IDLE;
                            o_SPI_busy <= '0';              -- reset busy flag
                            o_SPI_valid <= '1';
                        else
                            r_counter <= r_counter - 1;     -- decrease time counter
                        end if;
                    
                    when others =>
                        r_SPI_state <= t_IDLE;
                    
                end case;
            end if;
        end if;
    end process;
    
    o_MOSI <= r_shift_register(31);
    o_data <= r_MISO_data;

end Behavioral;
