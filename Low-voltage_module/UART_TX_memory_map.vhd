library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;


entity UART_TX_memory_map is
    port(
        -- i_clk                - system clock
        -- i_rst                - system reset
        -- i_begin              - goes to logic 1 for 1 clock period when transmission of data is supposed to start
        -- o_reg_G              - register for communication
        -- o_reg_H              - register for control
        -- o_reg_I              - register for voltage
        -- o_reg_J              - register for frequency
        -- i_name               - name of the module
        -- o_TX_pin             - output pin of UART transmitter
        -- o_TX_memory_busy     - busy flag (0 = not busy, 1 = busy)
        
        i_clk               : in    std_logic;
        i_rst               : in    std_logic;
        
        i_begin             : in    std_logic;
        i_reg_G             : in    std_logic_vector(15 downto 0);
        i_reg_H             : in    std_logic_vector(15 downto 0);
        i_reg_I             : in    std_logic_vector(31 downto 0);
        i_reg_J             : in    std_logic_vector(31 downto 0);
        i_name              : in    std_logic_vector(31 downto 0);
        
        o_TX_pin            : out   std_logic;
        o_TX_memory_busy    : out   std_logic
        );
end UART_TX_memory_map;

architecture Behavioral of UART_TX_memory_map is

    component UART_TX
        port(
        i_clk       : in    std_logic;
        i_rst       : in    std_logic;
        
        i_TX_begin  : in    std_logic;
        i_TX_data   : in    std_logic_vector(7 downto 0);
        
        o_TX_pin    : out   std_logic;
        o_TX_busy   : out   std_logic
        );
    end component;
    
    -- C_COUNTER_MAX                    - maximum value of counter
    
    constant    C_COUNTER_MAX           : integer                       := 40;
    
    -- r_TX_memory_state                - state of UART transmitter memory map
    -- r_TX_busy                        - busy flag (0 = not busy, 1 = busy)
    -- r_send_byte                      - goes to logic 1 when transmission of byte is supposed to begin
    -- r_byte_counter                   - counter of bytes in output string, goes from 0 to C_COUNTER_MAX
    -- r_byte                           - byte of data to be transmitted
    
    type        t_TX_memory_state is (t_IDLE, t_WAITING, t_SEND_DATA);
    
    signal      r_TX_memory_state       : t_TX_memory_state                 := t_IDLE;
    signal      r_TX_busy               : std_logic                         := '0';
    signal      r_send_byte             : std_logic                         := '0';
    signal      r_byte_counter          : integer range 0 to C_COUNTER_MAX  := 0;
    signal      r_byte                  : std_logic_vector(7 downto 0)      := (others => '0');
    
    -- function to convert hexadecimal number to ASCII character
    function f_HEX_to_ASCII(r_bits : in std_logic_vector(3 downto 0))
        return std_logic_vector is
    begin
        case r_bits is
            when X"A" => return X"41";
            when X"B" => return X"42";
            when X"C" => return X"43";
            when X"D" => return X"44";
            when X"E" => return X"45";
            when X"F" => return X"46";
            when others => return X"3" & r_bits;
        end case;
    end function;

begin

    -- process p_UART_TX_memory_state_machine controlls transmission of data to UART_TX
    -- system waits for i_begin pulse from superior code, then switches states between t_WAITING_1, t_WAITING_2 and t_SEND_DATA
    -- after all data are send, process returns to t_IDLE state
    p_UART_TX_memory_state_machine : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_TX_memory_state <= t_IDLE;
                o_TX_memory_busy <= '0';
            else
                case r_TX_memory_state is
                    -- =======================================
                    -- wait for i_begin signal from main block
                    when t_IDLE =>
                        if (i_begin = '1') then
                            r_TX_memory_state <= t_WAITING;
                            o_TX_memory_busy <= '1';
                        else
                            o_TX_memory_busy <= '0';
                        end if;
                    
                    -- =================================================
                    -- wait until UART_TX is ready for next transmission
                    when t_WAITING =>
                        if ((r_TX_busy = '0') and (r_send_byte /= '1') and (r_byte_counter < C_COUNTER_MAX)) then
                            r_TX_memory_state <= t_SEND_DATA;
                        elsif (r_byte_counter = C_COUNTER_MAX) then
                            r_TX_memory_state <= t_IDLE;
                        end if;
                    
                    -- ==================================================================================
                    -- send start signal and next byte to UART_RX and immediately go to t_WAITING_1 state
                    when t_SEND_DATA =>
                        r_TX_memory_state <= t_WAITING;
                        
                    when others =>
                        r_TX_memory_state <= t_IDLE;
                        
                end case;
            end if;
        end if;
    end process;

    -- process p_counter counts bytes which are being send to UART_TX
    -- all registers have 39 bytes together
    p_counter : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (r_TX_memory_state = t_IDLE) then
                r_byte_counter <= 0;
            elsif (r_TX_memory_state = t_SEND_DATA) then
                r_byte_counter <= r_byte_counter + 1;
            end if;
        end if;
    end process;
    
    -- process p_sending_data sends all bytes from all registers
    -- letter (name) of the register is send first and \n last
    p_sending_data : process(i_clk)
    begin
        if(rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_send_byte <= '0';
                r_byte <= (others => '0');
            else
                if (r_TX_memory_state = t_SEND_DATA) then
                    r_send_byte <= '1';
                    case r_byte_counter is
                        when 0 => r_byte <= X"40";      -- @
                        when 1 => r_byte <= i_name(31 downto 24);
                        when 2 => r_byte <= i_name(23 downto 16);
                        when 3 => r_byte <= i_name(15 downto 8);
                        when 4 => r_byte <= i_name(7 downto 0);
                        when 5 => r_byte <= X"0A";      -- \n
    
                        when 6 => r_byte <= X"47";      -- register G
                        when 7 => r_byte <= f_HEX_to_ASCII(i_reg_G(15 downto 12));
                        when 8 => r_byte <= f_HEX_to_ASCII(i_reg_G(11 downto 8));
                        when 9 => r_byte <= f_HEX_to_ASCII(i_reg_G(7 downto 4));
                        when 10 => r_byte <= f_HEX_to_ASCII(i_reg_G(3 downto 0));
                        when 11 => r_byte <= X"0A";     -- \n
                        
                        when 12 => r_byte <= X"48";     -- register H
                        when 13 => r_byte <= f_HEX_to_ASCII(i_reg_H(15 downto 12));
                        when 14 => r_byte <= f_HEX_to_ASCII(i_reg_H(11 downto 8));
                        when 15 => r_byte <= f_HEX_to_ASCII(i_reg_H(7 downto 4));
                        when 16 => r_byte <= f_HEX_to_ASCII(i_reg_H(3 downto 0));
                        when 17 => r_byte <= X"0A";     -- \n
                              
                        when 18 => r_byte <= X"49";     -- register I
                        when 19 => r_byte <= f_HEX_to_ASCII(i_reg_I(31 downto 28));
                        when 20 => r_byte <= f_HEX_to_ASCII(i_reg_I(27 downto 24));
                        when 21 => r_byte <= f_HEX_to_ASCII(i_reg_I(23 downto 20));
                        when 22 => r_byte <= f_HEX_to_ASCII(i_reg_I(19 downto 16));
                        when 23 => r_byte <= f_HEX_to_ASCII(i_reg_I(15 downto 12));
                        when 24 => r_byte <= f_HEX_to_ASCII(i_reg_I(11 downto 8));
                        when 25 => r_byte <= f_HEX_to_ASCII(i_reg_I(7 downto 4));
                        when 26 => r_byte <= f_HEX_to_ASCII(i_reg_I(3 downto 0));
                        when 27 => r_byte <= X"0A";     -- \n
                              
                        when 28 => r_byte <= X"4A";     -- register J
                        when 29 => r_byte <= f_HEX_to_ASCII(i_reg_J(31 downto 28));
                        when 30 => r_byte <= f_HEX_to_ASCII(i_reg_J(27 downto 24));
                        when 31 => r_byte <= f_HEX_to_ASCII(i_reg_J(23 downto 20));
                        when 32 => r_byte <= f_HEX_to_ASCII(i_reg_J(19 downto 16));
                        when 33 => r_byte <= f_HEX_to_ASCII(i_reg_J(15 downto 12));
                        when 34 => r_byte <= f_HEX_to_ASCII(i_reg_J(11 downto 8));
                        when 35 => r_byte <= f_HEX_to_ASCII(i_reg_J(7 downto 4));
                        when 36 => r_byte <= f_HEX_to_ASCII(i_reg_J(3 downto 0));
                        when 37 => r_byte <= X"0A";     -- \n
                        
                        when 38 => r_byte <= X"0A";     -- \n
                        when 39 => r_byte <= X"0D";     -- \r
                        
                        when others => r_byte <= X"3F"; -- ?
                        
                    end case;
                else
                    r_send_byte <= '0';
                end if;
            end if;
        end if;    
    end process;

    -- instance of UART_TX
    instance_UART_TX : UART_TX
        port map(
                i_clk => i_clk,
                i_rst => i_rst,
                i_TX_begin => r_send_byte,
                i_TX_data => r_byte,
                o_TX_pin => o_TX_pin,
                o_TX_busy => r_TX_busy
                );

end Behavioral;
