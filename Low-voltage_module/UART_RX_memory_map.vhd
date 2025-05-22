library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;


entity UART_RX_memory_map is
    port(
        -- i_clk            - system clock
        -- i_rst            - system reset
        -- i_RX_pin         - input pin of UART receiver
        -- o_reg_G          - register for communication
        -- o_reg_H          - register for control
        -- o_reg_I          - register for voltage
        -- o_reg_J          - register for frequency
        -- o_reg_G_strobe   - goes to logic 1 for 1 clk period when content of o_reg_G is updated
        -- o_reg_H_strobe   - goes to logic 1 for 1 clk period when content of o_reg_H is updated
        -- o_reg_I_strobe   - goes to logic 1 for 1 clk period when content of o_reg_I is updated
        -- o_reg_J_strobe   - goes to logic 1 for 1 clk period when content of o_reg_J is updated
        
        i_clk           : in    std_logic;
        i_rst           : in    std_logic;
        
        i_RX_pin        : in    std_logic;
        
        o_reg_G         : out   std_logic_vector(15 downto 0);
        o_reg_H         : out   std_logic_vector(15 downto 0);
        o_reg_I         : out   std_logic_vector(31 downto 0);
        o_reg_J         : out   std_logic_vector(31 downto 0);
        o_reg_G_strobe  : out   std_logic;
        o_reg_H_strobe  : out   std_logic;
        o_reg_I_strobe  : out   std_logic;
        o_reg_J_strobe  : out   std_logic
        );
end UART_RX_memory_map;

architecture Behavioral of UART_RX_memory_map is

    -- component UART_RX
    component UART_RX
        port (
            i_clk       : in    std_logic;
            i_rst       : in    std_logic;
            
            i_RX_pin    : in    std_logic;
            
            o_RX_data   : out   std_logic_vector(7 downto 0);
            o_RX_valid  : out   std_logic
            );
    end component;
    
    -- C_TIMER_FULL         - maximum value of timeout timer
    
    constant C_TIMER_FULL   : unsigned(23 downto 0) := X"FFFFFF";
                            
    -- r_RX_memory_state        - state of UART receiver memory map
    -- r_RX_valid               - signal from UART_RX that goes to logic 1 for 1 clock cycle when new byte of data is received
    -- r_RX_byte                - received byte from UART_RX
    -- r_timeout_timer          - timer that goes from C_TIMER_FULL to 0, if 0 is reached, r_RX_memory_state goes to t_IDLE state
    
    type t_RX_memory_state is (t_IDLE,      -- idle state, waiting for input from UART_RX
                            t_DIGIT_0,
                            t_DIGIT_1,
                            t_DIGIT_2,
                            t_DIGIT_3,
                            t_DIGIT_4,
                            t_DIGIT_5,
                            t_DIGIT_6,
                            t_DIGIT_7,
                            t_LF,           -- \n (but \n and \r can be received in any order)
                            t_CR            -- \r (but \n and \r can be received in any order)
                            );
    
    signal r_RX_memory_state    : t_RX_memory_state             := t_IDLE;
    signal r_RX_valid           : std_logic                     := '0';
    signal r_RX_byte            : std_logic_vector(7 downto 0)  := (others => '0');
    signal r_timeout_timer      : unsigned(23 downto 0)         := (others => '0');
    signal r_register           : std_logic_vector(7 downto 0)  := (others => '0');
    signal r_digit_0            : std_logic_vector(3 downto 0)  := (others => '0');
    signal r_digit_1            : std_logic_vector(3 downto 0)  := (others => '0');
    signal r_digit_2            : std_logic_vector(3 downto 0)  := (others => '0');
    signal r_digit_3            : std_logic_vector(3 downto 0)  := (others => '0');
    signal r_digit_4            : std_logic_vector(3 downto 0)  := (others => '0');
    signal r_digit_5            : std_logic_vector(3 downto 0)  := (others => '0');
    signal r_digit_6            : std_logic_vector(3 downto 0)  := (others => '0');
    signal r_digit_7            : std_logic_vector(3 downto 0)  := (others => '0');
    
    -- function to convert ASCII character to hexadecimal number
    function f_ASCII_to_HEX(r_byte : in std_logic_vector(7 downto 0))
        return std_logic_vector is
    begin
        case r_byte is
            when X"41" => return X"A";
            when X"42" => return X"B";
            when X"43" => return X"C";
            when X"44" => return X"D";
            when X"45" => return X"E";
            when X"46" => return X"F";
            when others => return r_byte(3 downto 0);
        end case;
    end function;
    
begin

    -- process p_UART_RX_memory_state_machine receives bytes of data from UART line and strores them in correct register
    -- first received byte represents name of register (G, H, I, J)
    -- rest are hexadecimal numbers representing data (G0000\n\r for 16-bit register)
    -- each byte is stored into 4-bit register (digit) and in the last state is stored into correct register
    -- each string send to FPGA by UART should end with \n and \r in any order
    p_UART_RX_memory_state_machine : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_RX_memory_state <= t_IDLE;
                r_timeout_timer <= C_TIMER_FULL;
                o_reg_G <= (others => '0');
                o_reg_H <= (others => '0');
                o_reg_I <= (others => '0');
                o_reg_J <= (others => '0');
                o_reg_G_strobe <= '0';
                o_reg_H_strobe <= '0';
                o_reg_I_strobe <= '0';
                o_reg_J_strobe <= '0';
                r_register <= X"00";
                r_digit_0 <= X"0";
                r_digit_1 <= X"0";
                r_digit_2 <= X"0";
                r_digit_3 <= X"0";
                r_digit_4 <= X"0";
                r_digit_5 <= X"0";
                r_digit_6 <= X"0";
                r_digit_7 <= X"0";
                
            else
                if (r_timeout_timer = X"000000") then
                    r_RX_memory_state <= t_IDLE;
                end if;
                
                case r_RX_memory_state is
                    -- ===============================================================================================
                    -- if r_RX_valid signal from UART_RX is received, identify correct register and go to DIGIT states
                    when t_IDLE =>
                        r_timeout_timer <= C_TIMER_FULL;
                        o_reg_G_strobe <= '0';
                        o_reg_H_strobe <= '0';
                        o_reg_I_strobe <= '0';
                        o_reg_J_strobe <= '0';
                        
                        if (r_RX_valid = '1') then
                            -- 16-bit registers G, H
                            if ((r_RX_byte = X"47") or (r_RX_byte = X"48")) then
                                r_register <= r_RX_byte;
                                r_RX_memory_state <= t_DIGIT_3;
                            --32-bit registers I, J
                            elsif ((r_RX_byte = X"49") or (r_RX_byte = X"4A")) then
                                r_register <= r_RX_byte;
                                r_RX_memory_state <= t_DIGIT_7;
                            end if;
                        end if;
                    
                    -- ==========================================
                    -- save hexadecimal number from received byte
                    when t_DIGIT_7 =>
                        r_timeout_timer <= r_timeout_timer - 1;     -- decrement timer
                        if (r_RX_valid = '1') then
                            if (((r_RX_byte >= X"30") and (r_RX_byte <= X"39")) or ((r_RX_byte >= X"41") and (r_RX_byte <= X"46"))) then
                                r_digit_7 <= f_ASCII_to_HEX(r_RX_byte);     -- number 0 - F
                                r_RX_memory_state <= t_DIGIT_6;
                            else
                                r_RX_memory_state <= t_IDLE;                -- anything else than 0 - F is error
                            end if;
                        end if;
                    
                    -- ==========================================
                    -- save hexadecimal number from received byte
                    when t_DIGIT_6 =>
                        r_timeout_timer <= r_timeout_timer - 1;     -- decrement timer
                        if (r_RX_valid = '1') then
                            if (((r_RX_byte >= X"30") and (r_RX_byte <= X"39")) or ((r_RX_byte >= X"41") and (r_RX_byte <= X"46"))) then
                                r_digit_6 <= f_ASCII_to_HEX(r_RX_byte);     -- number 0 - F
                                r_RX_memory_state <= t_DIGIT_5;
                            else
                                r_RX_memory_state <= t_IDLE;                -- anything else than 0 - F is error
                            end if;
                        end if;
                        
                    -- ==========================================
                    -- save hexadecimal number from received byte
                    when t_DIGIT_5 =>
                        r_timeout_timer <= r_timeout_timer - 1;     -- decrement timer
                        if (r_RX_valid = '1') then
                            if (((r_RX_byte >= X"30") and (r_RX_byte <= X"39")) or ((r_RX_byte >= X"41") and (r_RX_byte <= X"46"))) then
                                r_digit_5 <= f_ASCII_to_HEX(r_RX_byte);     -- number 0 - F
                                r_RX_memory_state <= t_DIGIT_4;
                            else
                                r_RX_memory_state <= t_IDLE;                -- anything else than 0 - F is error
                            end if;
                        end if;
                    
                    -- ==========================================
                    -- save hexadecimal number from received byte
                    when t_DIGIT_4 =>
                        r_timeout_timer <= r_timeout_timer - 1;     -- decrement timer
                        if (r_RX_valid = '1') then
                            if (((r_RX_byte >= X"30") and (r_RX_byte <= X"39")) or ((r_RX_byte >= X"41") and (r_RX_byte <= X"46"))) then
                                r_digit_4 <= f_ASCII_to_HEX(r_RX_byte);     -- number 0 - F
                                r_RX_memory_state <= t_DIGIT_3;
                            else
                                r_RX_memory_state <= t_IDLE;                -- anything else than 0 - F is error
                            end if;
                        end if;
                    
                    -- ==========================================
                    -- save hexadecimal number from received byte
                    when t_DIGIT_3 =>
                        r_timeout_timer <= r_timeout_timer - 1;     -- decrement timer
                        if (r_RX_valid = '1') then
                            if (((r_RX_byte >= X"30") and (r_RX_byte <= X"39")) or ((r_RX_byte >= X"41") and (r_RX_byte <= X"46"))) then
                                r_digit_3 <= f_ASCII_to_HEX(r_RX_byte);     -- number 0 - F
                                r_RX_memory_state <= t_DIGIT_2;
                            else
                                r_RX_memory_state <= t_IDLE;                -- anything else than 0 - F is error
                            end if;
                        end if;
                        
                    -- ==========================================
                    -- save hexadecimal number from received byte
                    when t_DIGIT_2 =>
                        r_timeout_timer <= r_timeout_timer - 1;     -- decrement timer
                        if (r_RX_valid = '1') then
                            if (((r_RX_byte >= X"30") and (r_RX_byte <= X"39")) or ((r_RX_byte >= X"41") and (r_RX_byte <= X"46"))) then
                                r_digit_2 <= f_ASCII_to_HEX(r_RX_byte);     -- number 0 - F
                                r_RX_memory_state <= t_DIGIT_1;
                            else
                                r_RX_memory_state <= t_IDLE;                -- anything else than 0 - F is error
                            end if;
                        end if;
                    
                    -- ==========================================
                    -- save hexadecimal number from received byte
                    when t_DIGIT_1 =>
                        r_timeout_timer <= r_timeout_timer - 1;     -- decrement timer
                        if (r_RX_valid = '1') then
                            if (((r_RX_byte >= X"30") and (r_RX_byte <= X"39")) or ((r_RX_byte >= X"41") and (r_RX_byte <= X"46"))) then
                                r_digit_1 <= f_ASCII_to_HEX(r_RX_byte);     -- number 0 - F
                                r_RX_memory_state <= t_DIGIT_0;
                            else
                                r_RX_memory_state <= t_IDLE;                -- anything else than 0 - F is error
                            end if;
                        end if;
                    
                    -- ==========================================
                    -- save hexadecimal number from received byte
                    when t_DIGIT_0 =>
                        r_timeout_timer <= r_timeout_timer - 1;     -- decrement timer
                        if (r_RX_valid = '1') then
                            if (((r_RX_byte >= X"30") and (r_RX_byte <= X"39")) or ((r_RX_byte >= X"41") and (r_RX_byte <= X"46"))) then
                                r_digit_0 <= f_ASCII_to_HEX(r_RX_byte);     -- number 0 - F
                                r_RX_memory_state <= t_LF;
                            else
                                r_RX_memory_state <= t_IDLE;                -- anything else than 0 - F is error
                            end if;
                        end if;
                    
                    -- =================================
                    -- wait until LF (or CR) is received
                    when t_LF =>
                        r_timeout_timer <= r_timeout_timer - 1;     -- decrement timer
                        if (r_RX_valid = '1') then
                            if (r_RX_byte = X"0A") or (r_RX_byte = X"0D") then
                                r_RX_memory_state <= t_CR;
                            else
                                r_RX_memory_state <= t_IDLE;
                            end if;
                        end if;
                        
                    -- ===================================================================================
                    -- wait until CR (or LF) is received as final byte and store all digits into registers
                    when t_CR =>
                        r_timeout_timer <= r_timeout_timer - 1;     -- decrement timer
                        if (r_RX_valid = '1') then
                            if ((r_RX_byte = X"0A") or (r_RX_byte = X"0D")) then
                                r_RX_memory_state <= t_IDLE;
                                case r_register is
                                    when X"47" =>
                                        o_reg_G <= r_digit_3 & r_digit_2 & r_digit_1 & r_digit_0;
                                        o_reg_G_strobe <= '1';
                                    when X"48" =>
                                        o_reg_H <= r_digit_3 & r_digit_2 & r_digit_1 & r_digit_0;
                                        o_reg_H_strobe <= '1';
                                    when X"49" =>
                                        o_reg_I <= r_digit_7 & r_digit_6 & r_digit_5 & r_digit_4 &
                                                    r_digit_3 & r_digit_2 & r_digit_1 & r_digit_0;
                                        o_reg_I_strobe <= '1';
                                    when X"4A" =>
                                        o_reg_J <= r_digit_7 & r_digit_6 & r_digit_5 & r_digit_4 &
                                                    r_digit_3 & r_digit_2 & r_digit_1 & r_digit_0;
                                        o_reg_J_strobe <= '1';
                                    when others =>
                                end case;
                            else
                                r_RX_memory_state <= t_IDLE;
                            end if;
                        end if;
                    
                    when others =>
                        r_RX_memory_state <= t_IDLE;
                        
                end case;
            end if;
        end if;
    end process;

-- instance of UART_RX
instance_UART_RX : UART_RX
    port map(
    i_clk => i_clk,
    i_rst => i_rst,
    i_RX_pin => i_RX_pin,
    o_RX_data => r_RX_byte,
    o_RX_valid => r_RX_valid
    );

end Behavioral;
