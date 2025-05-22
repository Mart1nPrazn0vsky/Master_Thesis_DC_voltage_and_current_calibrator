library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.MATH_REAL.ALL;


entity main is
    generic(
        -- G_CLOCK_FREQ     - FPGA clock frequency
        -- G_DITH_FREQ      - sampling frequency during DC mode with dithering
        -- G_AC_GEN_FREQ    - sampling frequency during AC mode
        
        G_CLOCK_FREQ        : real      := 12.0e6;
        G_DITH_FREQ         : positive  := 1000;
        G_AC_GEN_FREQ       : positive  := 100000
        );
    port(
        -- i_clk            - system clock
        -- i_rst            - system reset
        -- i_UART_RX_pin    - input pin of UART receiver
        -- i_SPI_MISO_pin   - SPI MISO pin
        -- i_ALARM_pin      - ALARM pin of DAC11001B
        -- o_UART_TX_pin    - output pin of UART transmitter
        -- o_SPI_MOSI_pin   - SPI MOSI pin
        -- o_SPI_CS_pin     - SPI CS pin
        -- o_SPI_SCLK_pin   - SPI SCLK pin
        -- o_LDAC_pin       - LDAC pin of DAC11001B
        -- o_CLR_pin        - CLR pin of DAC11001B
        -- o_R1S            - relay K1 set pin
        -- o_R1S            - relay K1 reset pin
        -- o_R2S            - relay K2 set pin
        -- o_R2R            - relay K2 reset pin
        -- o_R3S            - relay K3 set pin
        -- o_R3R            - relay K3 reset pin
        -- o_out_LED_1      - backlight for output binding posts 1
        -- o_out_LED_2      - backlight for output binding posts 2
        -- o_panel_LED_1G   - panel LED 1 green
        -- o_panel_LED_1R   - panel LED 1 red
        -- o_panel_LED_2G   - panel LED 2 green
        -- o_panel_LED_2R   - panel LED 2 red  
        -- o_panel_LED_3G   - panel LED 3 green
        -- o_panel_LED_3R   - panel LED 3 red  
        -- o_panel_LED_4G   - panel LED 4 green
        -- o_panel_LED_4R   - panel LED 4 red
        -- o_CK_out         - FPGA clock output (for frequency calibration)  
        
        i_clk               : in    std_logic;
        i_rst               : in    std_logic;
        
        i_UART_RX_pin       : in    std_logic;
        i_SPI_MISO_pin      : in    std_logic;
        i_ALARM_pin         : in    std_logic;
            
        o_UART_TX_pin       : out   std_logic;
        o_SPI_MOSI_pin      : out   std_logic;
        o_SPI_CS_pin        : out   std_logic;
        o_SPI_SCLK_pin      : out   std_logic;
        o_LDAC_pin          : out   std_logic;
        o_CLR_pin           : out   std_logic;
        o_R1S               : out   std_logic;
        o_R1R               : out   std_logic;
        o_R2S               : out   std_logic;
        o_R2R               : out   std_logic;
        o_R3S               : out   std_logic;
        o_R3R               : out   std_logic;
        o_out_LED_1         : out   std_logic;
        o_out_LED_2         : out   std_logic;
        o_panel_LED_1G      : out   std_logic;
        o_panel_LED_1R      : out   std_logic;
        o_panel_LED_2G      : out   std_logic;
        o_panel_LED_2R      : out   std_logic;
        o_panel_LED_3G      : out   std_logic;
        o_panel_LED_3R      : out   std_logic;
        o_panel_LED_4G      : out   std_logic;
        o_panel_LED_4R      : out   std_logic;
        o_CLK_out           : out   std_logic
        );
end main;

architecture Behavioral of main is

    component UART_RX_memory_map
        port(
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
    end component;
    
    component UART_TX_memory_map
        port(
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
    end component;
    
    component SPI_master
        port(
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
    end component;
    
    component DDS
        port(
        i_clk               : in    std_logic;
        i_rst               : in    std_logic;
        i_begin             : in    std_logic;
        i_FTW               : in	unsigned(31 downto 0);
        i_amplitude         : in    unsigned(31 downto 0);
        o_ready             : out   std_logic;
        o_code              : out   std_logic_vector(19 downto 0)
        );
    end component;

    -- SYSTEM REGISTERS
    -- r_name           - "CLVB"
    -- r_reg_G          - communication register
    -- r_reg_H          - control register
    -- r_reg_I          - voltage register
    -- r_reg_J          - frequency register
    -- r_reg_G_strobe   - goes to logic 1 for 1 clock cycle when new data were received into r_reg_G
    -- r_reg_H_strobe   - goes to logic 1 for 1 clock cycle when new data were received into r_reg_H
    -- r_reg_I_strobe   - goes to logic 1 for 1 clock cycle when new data were received into r_reg_I
    -- r_reg_J_strobe   - goes to logic 1 for 1 clock cycle when new data were received into r_reg_J
    
    
    --  register G
    --  -----------------------------------------------------------------
	--  |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
	--  -----------------------------------------------------------------
	--  | - | - | - | - | - | - | - | - |             byte              |
	--  -----------------------------------------------------------------
	--  byte       - if byte = '?', send name + content of all registers
	
    --  register H
    --  -----------------------------------------------------------------
	--  |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
	--  -----------------------------------------------------------------
	--  | - |L4R|L4G|L3R|L3G|L2R|L2G|L1R|L1G|OL2|OL1|AC |DIT| R3| R2| R1|
	--  -----------------------------------------------------------------
	--  L4R        - LED 4 red (1 = LED ON, 0 = LED OFF)
	--  L4G        - LED 4 green (1 = LED ON, 0 = LED OFF)
	--  L3R        - LED 3 red (1 = LED ON, 0 = LED OFF)  
	--  L3G        - LED 3 green (1 = LED ON, 0 = LED OFF)
	--  L2R        - LED 2 red (1 = LED ON, 0 = LED OFF)  
	--  L2G        - LED 2 green (1 = LED ON, 0 = LED OFF)
	--  L1R        - LED 4 red (1 = LED ON, 0 = LED OFF)  
	--  L1G        - LED 4 green (1 = LED ON, 0 = LED OFF)
	--  OL2        - backlight for output binding posts 1 (1 = LED ON, 0 = LED OFF)
	--  OL1        - backlight for output binding posts 2 (1 = LED ON, 0 = LED OFF)
	--  AC         - 0 = AC mode OFF, 1 = AC mode ON
	--  DIT        - 0 = dithering OFF, 1 = dithering ON
	--  Rx         - 0 = relay in default state, 1 = relay switched
	
	--  register I
    --  ---------------------------------------------------------------------------------------------------------------------------------
	--  |31 |30 |29 |28 |27 |26 |25 |24 |23 |22 |21 |20 |19 |18 |17 |16 |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
	--  ---------------------------------------------------------------------------------------------------------------------------------
	--  | - | - | - | - | - | - | - | - |                                 voltage                                       |   dithering   |
	--  ---------------------------------------------------------------------------------------------------------------------------------
	--  voltage    - 20-bit code for DAC in DC mode, in DC mode with dithering top 20 bits of code, in AC mode amplitude (max. x80000)
	--  dithering  - 4 lowest bits of 24-bit code for DC generation when dithering is ON
	
	--  register J
    --  ---------------------------------------------------------------------------------------------------------------------------------
	--  |31 |30 |29 |28 |27 |26 |25 |24 |23 |22 |21 |20 |19 |18 |17 |16 |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
	--  ---------------------------------------------------------------------------------------------------------------------------------
	--  |                                                  frequency tuning word                                                        |
	--  ---------------------------------------------------------------------------------------------------------------------------------
	--  frequency tuning word - FTW, DDS adds FTW to phase accumulator after every sample (X"FFFFFFFF" = 360°)
	
	-- REGISTERS
	-- r_name                               -- name of the module (CLVB)
	-- r_reg_G                              -- 16-bit long register G (communication with control module)
	-- r_reg_H                              -- 16-bit long register H (control of CLVB module - mode, range, LEDs)
	-- r_reg_I                              -- 32-bit long register I (binary value of voltage)
	-- r_reg_J                              -- 32-bit long register J (frequency tuning word for generation of AC signal)
	-- r_reg_G_strobe                       -- goes to logic 1 for 1 clk period when content of o_reg_G is updated
	-- r_reg_H_strobe                       -- goes to logic 1 for 1 clk period when content of o_reg_H is updated
	-- r_reg_I_strobe                       -- goes to logic 1 for 1 clk period when content of o_reg_I is updated
	-- r_reg_J_strobe                       -- goes to logic 1 for 1 clk period when content of o_reg_J is updated
    
    signal      r_name                      : std_logic_vector(31 downto 0) := X"434C5642";
    signal      r_reg_G                     : std_logic_vector(15 downto 0);
    signal      r_reg_H                     : std_logic_vector(15 downto 0);
    signal      r_reg_I                     : std_logic_vector(31 downto 0);
    signal      r_reg_J                     : std_logic_vector(31 downto 0);
    signal      r_reg_G_strobe              : std_logic;
    signal      r_reg_H_strobe              : std_logic;
    signal      r_reg_I_strobe              : std_logic;
    signal      r_reg_J_strobe              : std_logic;
    
    -- UART COMMUNICATION
    -- r_CLVB_UART_state                    -- state of CLVB UART communication interface
    -- r_UART_TX_memory_busy                -- busy flag (0 = not busy, 1 = busy)
    -- r_UART_TX_begin                      -- goes to logic 1 for 1 clk perion to start UART transmission
    
    type        t_CLVB_UART_state is (t_IDLE, t_SEND);
    signal      r_CLVB_UART_state           : t_CLVB_UART_state             := t_IDLE;
    signal      r_UART_TX_memory_busy       : std_logic                     := '0';
    signal      r_UART_TX_begin             : std_logic                     := '0';
    
    -- SPI COMMUNICATION
    -- r_SPI_begin                          - goes to logic 1 for 1 clk perion to start SPI transmission
    -- r_SPI_valid                          - goes to logic 1 for 1 clock cycle when byte of data is received
    -- r_SPI_busy                           - busy flag (0 = not busy, 1 = busy)
    -- r_SPI_data_send                      - 32 bits of data to be send by SPI
    -- r_SPI_data_read                      - 32 bits of data received by SPI
    
    signal      r_SPI_begin                 : std_logic                     := '0';
    signal      r_SPI_valid                 : std_logic                     := '0';
    signal      r_SPI_busy                  : std_logic                     := '0';
    signal      r_SPI_data_send             : std_logic_vector(31 downto 0) := (others => '0');
    signal      r_SPI_data_read             : std_logic_vector(31 downto 0) := (others => '0');
    
    -- RELAYS
    -- C_TIME_DELAY_RELAYS                  - maximum value of r_time_counter_relays
    
    constant    C_TIME_DELAY_RELAYS         : positive                                  := positive(ceil(10.0e-3 * G_CLOCK_FREQ));
    
    -- r_CLVB_relays_state                  - state of relas K1, K2, K3
    -- r_time_counter_relays                - counter of clock cycles, goes from 0 to C_TIME_DELAY_RELAYS
    -- r_relays_position                    - most recent position of relays (0 = relay in default state, 1 = relay switched)
    -- r_relays_waiting                     - equals 1 if relays are waiting to settle after setting of resetting
    
    type        t_CLVB_relays_state is (t_INIT_RELAYS, t_IDLE, t_SET_RELAYS);
    signal      r_CLVB_relays_state         : t_CLVB_relays_state                       := t_INIT_RELAYS;
    signal      r_time_counter_relays       : integer range 0 to C_TIME_DELAY_RELAYS    := 0;
    signal      r_relays_position           : std_logic_vector(2 downto 0)              := "000";
    signal      r_relays_waiting            : std_logic                                 := '0';
    
    -- DIGITAL TO ANALOG CONVERTER DAC11001B AND SIGNAL GENERATION
    -- C_ADR_DAC_DATA                       - address of DAC_DATA register of DAC11001B
    -- C_ADR_CONFIG1                        - address of CONFIG1 register of DAC11001B
    -- C_ADR_DAC_CLEAR_DATA                 - address of DAC_CLEAR_DATA register of DAC11001B
    -- C_ADR_TRIGGER                        - address of TRIGGER register of DAC11001B
    -- C_ADR_STATUS                         - address of STATUS register of DAC11001B
    -- C_ADR_CONFIG2                        - address of CONFIG2 register of DAC11001B
    -- C_TLDACSL                            - LDAC high time specified by datasheed of DAC11001B
    -- C_TLDACW                             - LDAC low time specified by datasheed of DAC11001B
    -- C_TIME_COUNTER_DITH_MAX              - maximum value of r_time_counter_dith
    -- C_TIME_COUNTER_AC_GEN_MAX            - maximum value of r_time_counter_AC
    
    constant    C_ADR_DAC_DATA              : std_logic_vector(6 downto 0)                  := "0000001";
    constant    C_ADR_CONFIG1               : std_logic_vector(6 downto 0)                  := "0000010";
    constant    C_ADR_DAC_CLEAR_DATA        : std_logic_vector(6 downto 0)                  := "0000011";
    constant    C_ADR_TRIGGER               : std_logic_vector(6 downto 0)                  := "0000100";
    constant    C_ADR_STATUS                : std_logic_vector(6 downto 0)                  := "0000101";
    constant    C_ADR_CONFIG2               : std_logic_vector(6 downto 0)                  := "0000110";
    constant    C_TLDACSL                   : positive                                      := positive(ceil(50.0e-9 * G_CLOCK_FREQ));
    constant    C_TLDACW                    : positive                                      := positive(ceil(20.0e-9 * G_CLOCK_FREQ));
    constant    C_TIME_COUNTER_DITH_MAX     : natural                                       := natural(round(G_CLOCK_FREQ / real(G_DITH_FREQ))) - 1;
    constant    C_TIME_COUNTER_AC_MAX       : natural                                       := natural(round(G_CLOCK_FREQ / real(G_AC_GEN_FREQ))) - 1;
    
    -- r_CLVB_DAC_state                     - state of CLVB DAC state machine
    -- r_voltage_code                       - 20-bit vector of data to be generated during DC mode without dithering or AC mode
    -- r_voltage_code_last                  - most recent code generated during DC mode without dithering
    -- r_AC_mode                            - 0 = DC mode, 1 = AC mode
    -- r_dith_mode                          - 0 = dithering OFF, 1 = dithering ON
    -- r_dith_index                         - index of r_dith_vector
    -- r_dith_vector                        - 16-bit vector containing ratio of '0' and '1' (during dithering mode)
    -- r_dith_digits                        - lowest 4 digits of 24-bit code in dithering mode
    -- r_dith_digits_last                   - most recent lowest 4 digits of 24-bit code in dithering mode
    -- r_dith_code_up                       - 20-bit code to be send to DAC, when r_dith_index = 1
    -- r_dith_code_down                     - 20-bit code to be send to DAC, when r_dith_index = 0
    -- r_bits_CONFIG1                       - configuration bits for CONFIG1 register of DAC11001B
    -- r_bits_CONFIG2                       - configuration bits for CONFIG2 register of DAC11001B
    -- r_bits_TRIGGER                       - configuration bits for TRIGGER register of DAC11001B
    -- r_time_counter_SPI                   - counter of clock cycles, goes from 0 to C_TLDACSL or to C_TLDACW
    -- r_time_counter_dith                  - counter of clock cycles, goes from 0 to C_TIME_COUNTER_DITH_MAX
    -- r_time_counter_AC                    - counter of clock cycles, goes from 0 to C_TIME_COUNTER_AC_MAX
    -- r_DDS_begin                          - goes to logic 1 for one clock cycle as signal for DDS to start calculation of the next sample
    -- r_DDS_FTW                            - frequency tuning word - "step" which is added to DDS phase acumulator after every sample
    -- r_DDS_amplitude                      - amplitude of sine signal for DDS
    -- r_DDS_ready                          - ready flag for main code, logic 0 - not ready, logic 1 - ready
    -- r_DDS_result_code                    - 20 bit result code from DDS for DAC11001B
    
    type        t_CLVB_DAC_state is (t_SET_CONFIG1,             -- write data into CONFIG1 register
                                    t_SET_CONFIG2,              -- write data into CONFIG2 register
                                    t_SET_TRIGGER,              -- write data into TRIGGER register
                                    t_IDLE,                     -- idle state, waiting for different voltage request
                                    t_SET_DAC_CODE,             -- prepare data to be transmitted by SPI in different modes of operation
                                    t_INIT_DITH,                -- initialize dithering variables
                                    t_SPI_START,                -- start SPI transmission to DAC11001B
                                    t_SPI_WAIT,                 -- wait until SPI transmission is finished
                                    t_LDAC_HIGH_WAIT,           -- wait for amount of time specified by datasheet of DAC11001B with LDAC high
                                    t_LDAC_LOW,                 -- set LDAC to low as required during different modes of operation
                                    t_LDAC_LOW_WAIT             -- wait for amount of time specified by datasheet of DAC11001B with LDAC low
                                    );
    signal      r_CLVB_DAC_state            : t_CLVB_DAC_state                              := t_SET_CONFIG1;
    signal      r_voltage_code              : std_logic_vector(19 downto 0)                 := (others => '0');
    signal      r_voltage_code_last         : std_logic_vector(19 downto 0)                 := (others => '0');
    signal      r_AC_mode                   : std_logic                                     := '0';
    signal      r_dith_mode                 : std_logic                                     := '0';
    signal      r_voltage_code_dith         : std_logic_vector(23 downto 0)                 := (others => '0');
    signal      r_voltage_code_dith_last    : std_logic_vector(23 downto 0)                 := (others => '0');
    signal      r_dith_index                : integer range 0 to 15                         := 0;
    signal      r_dith_vector               : std_logic_vector(15 downto 0)                 := (others => '0');
    signal      r_dith_code_up              : std_logic_vector(19 downto 0)                 := (others => '0');
    signal      r_dith_code_down            : std_logic_vector(19 downto 0)                 := (others => '0');
    signal      r_bits_CONFIG1              : std_logic_vector(19 downto 0)                 := "00000000010001100000"; -- SDO disabled, enhanced THD
    signal      r_bits_CONFIG2              : std_logic_vector(19 downto 0)                 := "00000000000000000011"; -- maximum DAC update rate
    signal      r_bits_TRIGGER              : std_logic_vector(19 downto 0)                 := "00000000000000000000"; -- no software reset
    signal      r_time_counter_SPI          : integer range 0 to C_TLDACSL                  := 0;
    signal      r_time_counter_dith         : integer range 0 to C_TIME_COUNTER_DITH_MAX    := 0;
    signal      r_time_counter_AC           : integer range 0 to C_TIME_COUNTER_AC_MAX      := 0;
    
    signal      r_DDS_begin                 : std_logic                                     := '0';
    signal      r_DDS_FTW                   : unsigned(31 downto 0)                         := (others => '0');
    signal      r_DDS_amplitude             : unsigned(31 downto 0)                         := (others => '0');
    signal      r_DDS_ready                 : std_logic                                     := '0';
    signal      r_DDS_result_code           : std_logic_vector(19 downto 0)                 := (others => '0');
    
begin

    -- process p_dithering_time_counter counts FPGA clock cycles between 2 settings of DAC11001B during generation of DC signal with dithering
    -- when r_time_counter_dith is equal to 0, o_LDAC_pin is set to 0 to update DAC11001B output with latest code
    p_dithering_time_counter : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_time_counter_dith <= 0;
            else
                if (r_time_counter_dith = C_TIME_COUNTER_DITH_MAX) then
                    r_time_counter_dith <= 0;
                else 
                    r_time_counter_dith <= r_time_counter_dith + 1;
                end if;
            end if;
        end if;
    end process;
    
    -- process p_AC_generation_time_counter counts FPGA clock cycles between 2 settings of DAC11001B during generation of AC signal
    -- when r_time_counter_AC is equal to 0, o_LDAC_pin is set to 0 to update DAC11001B output with latest code
    p_AC_generation_time_counter : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_time_counter_AC <= 0;
            else
                if (r_time_counter_AC = C_TIME_COUNTER_AC_MAX) then
                    r_time_counter_AC <= 0;
                else 
                    r_time_counter_AC <= r_time_counter_AC + 1;
                end if;
            end if;
        end if;
    end process;

    -- process p_CLVB_UART_communication waits for 1 clock pulse of r_reg_G_strobe (change in r_reg_G)
    -- if "?" was received, content of all registers is supposed to be send
    -- process waits until UART_TX_memory_map is ready to begin transmission and then sends r_UART_TX_begin pulse
    p_CLVB_UART_communication : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_name <= X"434C5642";
                r_CLVB_UART_state <= t_IDLE;
                r_UART_TX_begin <= '0';
            else
                case r_CLVB_UART_state is
                    -- ======================================================================
                    -- waiting for r_reg_G_strobe pulse, if "?" is received, go to next state
                    when t_IDLE =>
                        r_UART_TX_begin <= '0';
                        if (r_reg_G_strobe = '1') then
                            if (r_reg_G(7 downto 0) = X"3F") then   -- if "?" was received into r_reg_G
                                r_CLVB_UART_state <= t_SEND;        -- go to t_SEND state
                            end if;
                        end if;
                        
                    -- ========================================================
                    -- wait until transmitter is ready, then send stating pulse
                    when t_SEND =>
                        if (r_UART_TX_memory_busy = '0') then
                            r_UART_TX_begin <= '1';                 -- start transmission
                            r_CLVB_UART_state <= t_IDLE;            -- go back to t_IDLE state
                        end if;
                        
                    when others =>
                        r_CLVB_UART_state <= t_IDLE;
                        
                end case;
            end if;
        end if;        
    end process;
    
    -- process p_CLVB_control waits for change in r_reg_H (control register)
    -- after new information is received, process sets all signals for control of CLVB
    p_CLVB_control : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_dith_mode <= '0';
                r_AC_mode <= '0';
                o_out_LED_1 <= '0';
                o_out_LED_2 <= '0';
                o_panel_LED_1G <= '0';
                o_panel_LED_1R <= '0';
                o_panel_LED_2G <= '0';
                o_panel_LED_2R <= '0';
                o_panel_LED_3G <= '0';
                o_panel_LED_3R <= '0';
                o_panel_LED_4G <= '0';
                o_panel_LED_4R <= '0';
                r_CLVB_relays_state <= t_INIT_RELAYS;
            else
                if (r_reg_H_strobe = '1') then
                    r_CLVB_relays_state <= t_SET_RELAYS;
                    r_dith_mode <= r_reg_H(3);
                    r_AC_mode <= r_reg_H(4);
                    o_out_LED_1 <= r_reg_H(5);
                    o_out_LED_2 <= r_reg_H(6);
                    o_panel_LED_1G <= r_reg_H(7);
                    o_panel_LED_1R <= r_reg_H(8);
                    o_panel_LED_2G <= r_reg_H(9);
                    o_panel_LED_2R <= r_reg_H(10);
                    o_panel_LED_3G <= r_reg_H(11);
                    o_panel_LED_3R <= r_reg_H(12);
                    o_panel_LED_4G <= r_reg_H(13);
                    o_panel_LED_4R <= r_reg_H(14);
                else
                    r_CLVB_relays_state <= t_IDLE;
                end if;
            end if;
        end if;
    end process;
    
    -- process p_CLVB_relays switches relays K1, K2, K3 which are used for range switching and disconnecting device output
    -- set and reset pins of relays are assigned with logical values only if relay is in different state than desired
    -- current state of relay is saved into r_relays_position after every change
    p_CLVB_relays : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_time_counter_relays <= C_TIME_DELAY_RELAYS;
                r_relays_waiting <= '0';
            else
                case r_CLVB_relays_state is
                    -- ==========================================================================
                    -- reset all relays into default state (signal +/- 2.2V, output disconnected)
                    when t_INIT_RELAYS =>
                        o_R1S <= '0';
                        o_R1R <= '1';
                        o_R2S <= '0';
                        o_R2R <= '1';
                        o_R3S <= '0';
                        o_R3R <= '1';
                        r_relays_waiting <= '1';
                        r_time_counter_relays <= C_TIME_DELAY_RELAYS;
                        
                    -- =================================================================================
                    -- if relays were switched, wait for 10 ms, then set all controll signals to logic 0
                    when t_IDLE =>
                        if (r_relays_waiting = '1') then
                            if (r_time_counter_relays = 0) then
                                o_R1S <= '0';
                                o_R1R <= '0';
                                o_R2S <= '0';
                                o_R2R <= '0';
                                o_R3S <= '0';
                                o_R3R <= '0';
                                r_relays_position(0) <= r_reg_H(0);        -- K2
                                r_relays_position(1) <= r_reg_H(1);        -- K3
                                r_relays_position(2) <= r_reg_H(2);        -- K1
                                r_relays_waiting <= '0';
                            else
                                r_time_counter_relays <= r_time_counter_relays - 1;
                            end if;
                        end if;
                    
                    -- ===============================================================================
                    -- check if current state is different than desired, if necessary set/reset relays
                    when t_SET_RELAYS =>
                        if (r_reg_H(2 downto 0) /= r_relays_position(2 downto 0)) then
                            if (r_reg_H(0) /= r_relays_position(0)) then
                                o_R1S <= r_reg_H(0);
                                o_R1R <= not r_reg_H(0);
                            end if;
                            if (r_reg_H(1) /= r_relays_position(1)) then
                                o_R2S <= r_reg_H(1);
                                o_R2R <= not r_reg_H(1);
                            end if;
                            if (r_reg_H(2) /= r_relays_position(2)) then
                                o_R3S <= r_reg_H(2);
                                o_R3R <= not r_reg_H(2);
                            end if;
                            r_relays_waiting <= '1';
                            r_time_counter_relays <= C_TIME_DELAY_RELAYS;
                        end if;
                    
                    when others =>
                    
                end case;
            end if;        
        end if;
    end process;
    
    -- process p_CLVB_AC_FTW assign most recent value of FTW to r_DDS_FTW
    -- this means that frequency can be changed automatically withou chaning any other register
    p_CLVB_AC_FTW : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_DDS_FTW <= (others => '0');
            else
                r_DDS_FTW <= unsigned(r_reg_J);
            end if;
        end if;
    end process;
    
    -- process p_CLVB_DAC controlls modes of operation of CLVB (DC mode with/without dithering, AC mode)
    -- after FPGA reset, process writes configuration bits into CONFIG1, CONFIG2 and TRIGGER registers of DAC11001B and then waits in idle state
    -- after new data are received into r_reg_I, process send correct code to DAC11001B by SPI interface
    -- DC mode without dithering - immediately goes to SPI transmission, immediate LADC low
    -- DC mode with dithering - immediately goes to SPI transmission, synchronous LDAC low
    -- AC mode - immediately goes to SPI transmission (new DDS sample calculation runs simultaniously), synchronous LDAC low
    p_CLVB_DAC : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_CLVB_DAC_state <= t_SET_CONFIG1;
                r_SPI_begin <= '0';
                r_time_counter_SPI <= 0;
                o_LDAC_pin <= '1';
                r_dith_index <= 0;
                r_bits_CONFIG1 <= "00000000010001100000";
                r_bits_CONFIG2 <= "00000000000000000011";
                r_bits_TRIGGER <= "00000000000000000000";
                
            else
                if (r_reg_I_strobe = '1') then
                    r_voltage_code <= r_reg_I(23 downto 4);         -- update r_voltage_code
                    r_voltage_code_dith <= r_reg_I(23 downto 0);    -- update r_voltage_code_dith
                    r_DDS_amplitude <= X"000" & unsigned(r_reg_I(23 downto 4));     -- AC signal amplitude (max = x80000)
                end if;
                
                case r_CLVB_DAC_state is
                    
                    -- ========================================
                    -- write correct data into CONFIG1 register
                    when t_SET_CONFIG1 =>
                        if ((r_SPI_busy = '0') and (r_SPI_begin /= '1')) then
                            r_SPI_data_send <= "0" & C_ADR_CONFIG1 & r_bits_CONFIG1 & "0000";
                            r_SPI_begin <= '1';
                            r_CLVB_DAC_state <= t_SET_CONFIG2;
                        else
                            r_SPI_begin <= '0';
                        end if;
                        
                    -- ========================================
                    -- write correct data into CONFIG2 register
                    when t_SET_CONFIG2 =>
                        if ((r_SPI_busy = '0') and (r_SPI_begin /= '1')) then
                            r_SPI_data_send <= "0" & C_ADR_CONFIG2 & r_bits_CONFIG2 & "0000";
                            r_SPI_begin <= '1';
                            r_CLVB_DAC_state <= t_SET_TRIGGER;
                        else
                            r_SPI_begin <= '0';
                        end if;
                    
                    -- ========================================
                    -- write correct data into TRIGGER register
                    when t_SET_TRIGGER =>
                        if ((r_SPI_busy = '0') and (r_SPI_begin /= '1')) then
                            r_SPI_data_send <= "0" & C_ADR_TRIGGER & r_bits_TRIGGER & "0000";
                            r_SPI_begin <= '1';
                            r_CLVB_DAC_state <= t_IDLE;
                        else
                            r_SPI_begin <= '0';
                        end if;
                    
                    -- ====================================================================================================
                    -- waiting for change in voltage code, after that, DAC is set as required by current mode of generation
                    when t_IDLE =>
                        r_SPI_begin <= '0';
                        o_LDAC_pin <= '1';
                        
                        ---------------- DC mode, dithering OFF ----------------
                        if ((r_AC_mode = '0') and (r_dith_mode = '0')) then
                            if (r_voltage_code_last /= r_voltage_code) then
                                r_CLVB_DAC_state <= t_SPI_START;
                            end if;
                        ---------------- DC mode, dithering ON ----------------
                        elsif ((r_AC_mode = '0') and (r_dith_mode = '1')) then
                            if (r_voltage_code_dith_last /= r_voltage_code_dith) then
                                r_CLVB_DAC_state <= t_INIT_DITH;
                            else
                                r_CLVB_DAC_state <= t_SPI_START;    -- in dithering mode, go directly to SPI transmission
                            end if;
                        ---------------- AC mode ----------------
                        else
                            if (r_DDS_ready = '1') then
                                r_DDS_begin <= '1';
                                r_voltage_code <= r_DDS_result_code;
                                r_CLVB_DAC_state <= t_SPI_START;
                            end if;
                        end if;
                    
                    -- ===================================================================================================
                    -- set correct up and down codes and r_dith_vector which sets ratio of generation of up and down codes
                    when t_INIT_DITH =>
                        if (r_voltage_code = X"FFFFF") then
                            r_dith_code_up <= r_voltage_code;               -- if maximum value (2^20) was received
                        else
                            r_dith_code_up <= std_logic_vector(unsigned(r_voltage_code) + 1);   -- any other value
                        end if;
                        r_dith_code_down <= r_voltage_code;
                        case r_voltage_code_dith(3 downto 0) is
                            when X"0" => r_dith_vector <= "0000000000000000";
                            when X"1" => r_dith_vector <= "1000000000000000";
                            when X"2" => r_dith_vector <= "1000000010000000";
                            when X"3" => r_dith_vector <= "1000010000100000";
                            when X"4" => r_dith_vector <= "1000100010001000";
                            when X"5" => r_dith_vector <= "1001001001001000";
                            when X"6" => r_dith_vector <= "1010010010010010";
                            when X"7" => r_dith_vector <= "1010010101001010";
                            when X"8" => r_dith_vector <= "1010101010101010";
                            when X"9" => r_dith_vector <= "1110101010101010";
                            when X"A" => r_dith_vector <= "1101101101101010";
                            when X"B" => r_dith_vector <= "1101101101101110";
                            when X"C" => r_dith_vector <= "1110111011101110";
                            when X"D" => r_dith_vector <= "1111101111011110";
                            when X"E" => r_dith_vector <= "1111111011111110";
                            when X"F" => r_dith_vector <= "1111111111111110";
                            when others =>
                        end case;
                        r_voltage_code_dith_last <= r_voltage_code_dith;
                        r_dith_index <= 0;
                        r_CLVB_DAC_state <= t_IDLE;
                    
                    -- =================================================================================
                    -- create 32-bit data vector, wait until SPI is not busy and then start transmission
                    when t_SPI_START =>
                        r_DDS_begin <= '0';
                        
                        if ((r_SPI_busy = '0') and (r_SPI_begin /= '1')) then
                            ---------------- DC mode, dithering OFF ----------------
                            if ((r_AC_mode = '0') and (r_dith_mode = '0')) then                     
                                r_SPI_data_send <= "0" & C_ADR_DAC_DATA & r_voltage_code & "0000";
                            ---------------- DC mode, dithering ON ----------------
                            elsif ((r_AC_mode = '0') and (r_dith_mode = '1')) then
                                if (r_dith_vector(r_dith_index) = '0') then
                                    r_SPI_data_send <= "0" & C_ADR_DAC_DATA & r_dith_code_down & "0000";
                                else
                                    r_SPI_data_send <= "0" & C_ADR_DAC_DATA & r_dith_code_up & "0000";
                                end if;
                                if (r_dith_index = 15) then
                                    r_dith_index <= 0;
                                else
                                    r_dith_index <= r_dith_index + 1;
                                end if;
                            ---------------- AC mode ----------------
                            else
                                r_SPI_data_send <= "0" & C_ADR_DAC_DATA & r_voltage_code & "0000";
                            end if;
                            r_SPI_begin <= '1';
                            r_CLVB_DAC_state <= t_SPI_WAIT;
                        end if;
                        
                    -- =======================================
                    -- wait until SPI transmission is finished
                    when t_SPI_WAIT =>
                        r_SPI_begin <= '0';
                        if (r_SPI_valid = '1') then
                            r_CLVB_DAC_state <= t_LDAC_HIGH_WAIT;
                            r_time_counter_SPI <= C_TLDACSL;
                        end if;
                    
                    -- ========================================================================================
                    -- wait for time specified in DAC11001B datasheet after SPI transmission to set LDAC to low
                    when t_LDAC_HIGH_WAIT =>
                        if (r_time_counter_SPI = 0) then
                            r_CLVB_DAC_state <= t_LDAC_LOW;
                        else
                            r_time_counter_SPI <= r_time_counter_SPI - 1;
                        end if;
                    
                    -- ==============================================================================================
                    -- in DC mode without dithering, set LDAC to logic 0 immediately, in other modes wait for counter
                    when t_LDAC_LOW =>
                        ---------------- DC mode, dithering OFF ----------------
                        if ((r_AC_mode = '0') and (r_dith_mode = '0')) then
                            o_LDAC_pin <= '0';          -- in DC mode without dithering, update DAC output immediately
                            r_CLVB_DAC_state <= t_LDAC_LOW_WAIT;
                            r_time_counter_SPI <= C_TLDACW;
                        ---------------- DC mode, dithering ON ----------------
                        elsif ((r_AC_mode = '0') and (r_dith_mode = '1')) then
                            if (r_time_counter_dith = 0) then
                                o_LDAC_pin <= '0';      -- in DC mode with dithering, wait for dithering timer
                                r_CLVB_DAC_state <= t_LDAC_LOW_WAIT;
                                r_time_counter_SPI <= C_TLDACW;
                            end if;
                        ---------------- AC mode ----------------
                        else
                            if (r_time_counter_AC = 0) then
                                o_LDAC_pin <= '0';          -- in AC mode wait for AC timer
                                r_CLVB_DAC_state <= t_LDAC_LOW_WAIT;
                                r_time_counter_SPI <= C_TLDACW;
                            end if;
                        end if;
                    
                    -- ===============================================================
                    -- wait for time specified in DAC11001B datasheet when LDAC is low
                    when t_LDAC_LOW_WAIT =>
                        if (r_time_counter_SPI = 0) then
                            r_CLVB_DAC_state <= t_IDLE;
                            r_voltage_code_last <= r_voltage_code;
                        else
                            r_time_counter_SPI <= r_time_counter_SPI - 1;
                        end if;
                        
                    when others =>
                    
                end case;
            end if;
        end if;
    end process;
    
    
    o_CLR_pin <= '1';
    o_CLK_out <= i_clk;
    

    -- instance of UART_RX_memory_map
    instance_UART_RX_memory_map : UART_RX_memory_map
        port map(
            i_clk => i_clk,
            i_rst => i_rst,
            i_RX_pin => i_UART_RX_pin,
            o_reg_G => r_reg_G,
            o_reg_H => r_reg_H,
            o_reg_I => r_reg_I,
            o_reg_J => r_reg_J,
            o_reg_G_strobe => r_reg_G_strobe,
            o_reg_H_strobe => r_reg_H_strobe,
            o_reg_I_strobe => r_reg_I_strobe,
            o_reg_J_strobe => r_reg_J_strobe
            );
    
    -- instance of UART_TX_memory_map
    instance_UART_TX_memory_map : UART_TX_memory_map
        port map(
            i_clk => i_clk,
            i_rst => i_rst,
            i_begin => r_UART_TX_begin,
            i_reg_G => r_reg_G,
            i_reg_H => r_reg_H,
            i_reg_I => r_reg_I,
            i_reg_J => r_reg_J,
            i_name => r_name,
            o_TX_pin => o_UART_TX_pin,
            o_TX_memory_busy => r_UART_TX_memory_busy
            );
    
    -- instance of SPI_master
    instance_SPI_master : SPI_master
        port map(
            i_clk => i_clk,
            i_rst => i_rst,
            i_begin => r_SPI_begin,
            i_data => r_SPI_data_send,
            i_MISO => i_SPI_MISO_pin,
            o_MOSI => o_SPI_MOSI_pin,
            o_CS => o_SPI_CS_pin,
            o_SCLK => o_SPI_SCLK_pin,
            o_SPI_valid => r_SPI_valid,
            o_SPI_busy => r_SPI_busy,
            o_data => r_SPI_data_read
            );
            
    -- instance of DDS
    instance_DDS : DDS
        port map(
            i_clk => i_clk,
            i_rst => i_rst,
            i_begin => r_DDS_begin,
            i_FTW => r_DDS_FTW,
            i_amplitude => r_DDS_amplitude,
            o_ready => r_DDS_ready,
            o_code => r_DDS_result_code
            );

end Behavioral;
