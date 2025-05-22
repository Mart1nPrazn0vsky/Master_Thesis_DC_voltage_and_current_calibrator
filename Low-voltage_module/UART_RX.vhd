library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.MATH_REAL.ALL;


entity UART_RX is
    generic(
        -- G_CLOCK_FREQ - FPGA clock frequency
        -- G_BAUD_RATE  - UART baud rate
        
        G_CLOCK_FREQ    : real      := 12.0e6;
        G_BAUD_RATE     : positive  := 9600
        );
    port(
        -- i_clk        - system clock
        -- i_rst        - system reset
        -- i_RX_pin     - input pin of UART receiver
        -- o_RX_data    - 1 byte of received data
        -- o_RX_valid   - goes to logic 1 for 1 clock cycle when byte of data is received
        
        i_clk       : in    std_logic;
        i_rst       : in    std_logic;
        
        i_RX_pin    : in    std_logic;
        
        o_RX_data   : out   std_logic_vector(7 downto 0);
        o_RX_valid  : out   std_logic
        );
end UART_RX;

architecture Behavioral of UART_RX is

    -- C_CLK_DIVIDER        - FPGA clock frequency / UART baud rate
    -- C_CLK_DIVIDER_WIDTH  - number of bits needed to store number in range 0 to C_CLK_DIVIDER

    constant C_CLK_DIVIDER          : positive  := positive(round(G_CLOCK_FREQ / real(G_BAUD_RATE)));
    constant C_CLK_DIVIDER_WIDTH    : positive  := positive(ceil(log2(real(C_CLK_DIVIDER))));
    
    -- r_RX_state           - state of UART receiver
    -- r_clk_counter        - counter of UART clock, goes from 0 to C_CLK_DIVIDER -1
    -- r_data               - 8 bits of received data
    -- r_data_counter       - counter for 8 bits of received data
    -- r_RX_pin_state       - state of input RX wire is saved into this signal
    -- r_RX_falling_edge    - if falling edge occured, this signal is logic 1
    
    type t_RX_state is (t_RX_IDLE, t_RX_DATA, t_RX_STOP);
    
    signal r_RX_state           : t_RX_state                                    := t_RX_IDLE;
    signal r_clk_counter        : unsigned(C_CLK_DIVIDER_WIDTH - 1 downto 0)    := (others => '0');
    signal r_data               : std_logic_vector(7 downto 0)                  := (others => '0');
    signal r_data_counter       : unsigned(7 downto 0)                          := (others => '0');
    signal r_RX_pin_state       : std_logic                                     := '0';
    signal r_RX_falling_edge    : std_logic                                     := '0';
    
begin

    r_RX_falling_edge <= (not i_RX_pin) and r_RX_pin_state;
    
    -- p_UART_RX_counter controls r_clk_counter which goes from 0 to C_CLK_DIVIDER - 1 (1 period of UART)
    -- if falling edge occurs in t_RX_IDLE state of the line, counter is set to 0
    -- if not, process in incrementing counter until it reaches C_CLK_DIVIDER - 1
    p_UART_RX_counter : process(i_clk)
    begin
        
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_clk_counter <= (others => '0');
                r_RX_pin_state <= i_RX_pin;
            else
                r_RX_pin_state <= i_RX_pin;     -- store RX pin state
                
                -- if bus is idle and falling edge occured
                if ((r_RX_state = t_RX_IDLE) and (r_RX_falling_edge = '1')) then
                    r_clk_counter <= (others => '0');
                else
                    -- if counter reached top, set it to zero
                    if (r_clk_counter = C_CLK_DIVIDER - 1) then
                        r_clk_counter <= (others => '0');
                    -- if counter in not at top, increment it
                    else
                        r_clk_counter <= r_clk_counter + 1;
                    end if;
                end if;
            end if;
        end if;
    end process;
    
    -- p_UART_receiver is in t_RX_IDLE state until first falling edge of i_RX_pin
    -- then, it switches to t_RX_DATA state, where it checks RX wire and saves data into r_data
    -- after 8 received bits are saved, state switches to t_RX_STOP
    -- in t_RX_STOP state, o_RX_valid goes to logic 1 for 1 clock cycle as indication for superior block
    p_UART_receiver : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            -- reset is active high
            if (i_rst = '1') then
                o_RX_valid <= '0';
                r_RX_state <= t_RX_IDLE;
                r_data <= (others => '0');
                r_data_counter <= (others => '0');
            else
                o_RX_valid <= '0';
                
                case r_RX_state is
                    --=====================================================
                    -- in t_RX_IDLE state, line is waiting for falling edge
                    when t_RX_IDLE =>
                        r_data_counter <= (others => '0');              -- in t_RX_IDLE state, cunter is allways 0
                        -- check middle of possible start bit
                        if ((r_RX_pin_state = '0') and (r_clk_counter = C_CLK_DIVIDER / 2 - 1)) then
                            r_RX_state <= t_RX_DATA;                    -- if start bit occurs, set next state
                        end if;
                    
                    --==========================================================================
                    -- in t_RX_DATA state, receiver is sampling data in the middle of UART pulse
                    when t_RX_DATA =>
                        if (r_clk_counter = C_CLK_DIVIDER / 2 - 1) then     -- check logic signal in the middle of data pulse
                            r_data <= r_RX_pin_state & r_data(7 downto 1);  -- make bit shift and save new bit into r_data
                            if (r_data_counter = 7) then
                                r_RX_state <= t_RX_STOP;                    -- go to nex state
                            else
                                r_data_counter <= r_data_counter + 1;       -- increment data counter
                            end if;
                        end if;
                    
                    --=======================================================================================================
                    -- in t_RX_STOP state, receiver sends 1 pulse in o_RX_valid as indication that new byte has been received
                    when t_RX_STOP =>
                        if (r_clk_counter = C_CLK_DIVIDER / 2 - 1) then     -- check logic signal in the middle of stop bit pulse
                            if (r_RX_pin_state = '1') then                  -- stop bit must be logic 1
                                o_RX_valid <= '1';
                            end if;
                            r_RX_state <= t_RX_IDLE;                        -- go to t_RX_IDLE state
                        end if;
                        
                    when others =>
                        r_RX_state <= t_RX_IDLE;
                    
                end case;
            end if;    
        end if;
    end process;
    
    o_RX_data <= r_data;

end Behavioral;
