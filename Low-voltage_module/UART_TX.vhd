library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.MATH_REAL.ALL;


entity UART_TX is
    generic(
        -- G_CLOCK_FREQ - FPGA clock frequency
        -- G_BAUD_RATE  - UART baud rate
        
        G_CLOCK_FREQ    : real      := 12.0e6;
        G_BAUD_RATE     : positive  := 9600
        );
    port(
        -- i_clk        - system clock
        -- i_rst        - system reset
        -- i_TX_begin   - goes to logic 1 for 1 clk perion when superior code wants to transmitt byte of data
        -- i_TX_data    - byte of data to be transmitted
        -- o_TX_pin     - output pin of UART transmitter
        -- o_TX_busy    - busy flag (0 = not busy, 1 = busy)
        
        i_clk       : in    std_logic;
        i_rst       : in    std_logic;
        
        i_TX_begin  : in    std_logic;
        i_TX_data   : in    std_logic_vector(7 downto 0);
        
        o_TX_pin    : out   std_logic;
        o_TX_busy   : out   std_logic
        );
end UART_TX;

architecture Behavioral of UART_TX is

    -- C_CLK_DIVIDER        - FPGA clock frequency / UART baud rate
    -- C_CLK_DIVIDER_WIDTH  - number of bits needed to store number in range 0 to C_CLK_DIVIDER

    constant C_CLK_DIVIDER          : positive  := positive(round(G_CLOCK_FREQ / real(G_BAUD_RATE)));
    constant C_CLK_DIVIDER_WIDTH    : positive  := positive(ceil(log2(real(C_CLK_DIVIDER))));
    
    -- r_TX_state           - state of UART transmitter
    -- r_clk_counter        - counter of CLK periods, goes from 0 to C_CLK_DIVIDER
    -- r_bit_index          - index of 8 bits of output data
    
    type t_TX_state is (t_TX_IDLE, t_TX_START, t_TX_DATA, t_TX_STOP);
    
    signal r_TX_state       : t_TX_state                                    := t_TX_IDLE;
    signal r_clk_counter    : unsigned(C_CLK_DIVIDER_WIDTH - 1 downto 0)    := (others => '0');
    signal r_bit_index      : natural range 0 to 7                          := 0;

begin

    -- p_UART_transmitter works in 4 states - idle, start bit, data bits, stop bit
    -- in idle state, transmitter waits for new byte and 1 clock pulse (i_TX_begin) from superior block
    -- after that, transmitter sends 1 start bit + 8 data bits + 1 stop bit and returns to idle state
    p_UART_transmitter : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_TX_state <= t_TX_IDLE;
                o_TX_busy <= '0';
                r_clk_counter <= (others => '0');
                r_bit_index <= 0;
                o_TX_pin <= '1';
            else
                case r_TX_state is
                    -- ==================================================================================
                    -- in t_TX_IDLE state, line is in logic 1, trasmitter is waiting for i_TX_begin pulse
                    when t_TX_IDLE =>
                        if (i_TX_begin = '1') then          -- start signal received
                            r_TX_state <= t_TX_start;
                            o_TX_busy <= '1';               -- set busy flag
                        else
                            r_TX_state <= t_TX_IDLE;
                            o_TX_busy <= '0';
                            r_clk_counter <= (others => '0');
                            r_bit_index <= 0;
                            o_TX_pin <= '1';
                        end if;
                        
                    -- ==========================================================
                    -- in t_TX_START state, transmitter sends logic 0 (start bit)
                    when t_TX_START =>
                        o_TX_pin <= '0';                            -- start bit is logic 0
                        if (r_clk_counter = C_CLK_DIVIDER - 1) then
                            r_clk_counter <= (others => '0');
                            r_TX_state <= t_TX_DATA;
                        else
                            r_clk_counter <= r_clk_counter + 1;
                        end if;
                    
                    -- =================================================================================
                    -- in t_TX_DATA state, transmitter is transmitting all bits of data byte (LSB first)
                    when t_TX_DATA =>
                        o_TX_pin <= i_TX_data(r_bit_index);
                        -- when transmitter did not send all bits yet
                        if ((r_clk_counter = C_CLK_DIVIDER - 1) and (r_bit_index < 7)) then
                            r_clk_counter <= (others => '0');
                            r_bit_index <= r_bit_index + 1;         -- increment bit counter
                        --when transmitter sent all bits
                        elsif ((r_clk_counter = C_CLK_DIVIDER - 1) and (r_bit_index = 7)) then
                            r_clk_counter <= (others => '0');
                            r_TX_state <= t_TX_STOP;
                        else
                            r_clk_counter <= r_clk_counter + 1;
                        end if;
                        
                    -- ==================================================================================
                    -- in t_TX_STOP state, transmitter sets line do logic 1 and switch to t_TX_IDLE state
                    when t_TX_STOP =>
                        o_TX_pin <= '1';                            -- stop bit is logic 1
                        if (r_clk_counter = C_CLK_DIVIDER - 1) then
                            r_TX_state <= t_TX_IDLE;
                            r_clk_counter <= (others => '0');
                            r_bit_index <= 0;
                        else
                            r_clk_counter <= r_clk_counter + 1;
                        end if;
                    
                    when others =>
                        r_TX_state <= t_TX_IDLE;
                
                end case;
            end if;
        end if;
    end process;

end Behavioral;
