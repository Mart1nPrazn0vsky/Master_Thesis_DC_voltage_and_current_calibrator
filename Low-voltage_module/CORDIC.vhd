library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.MATH_REAL.ALL;


entity CORDIC is
    port(
        -- i_clk            - system clock
        -- i_rst            - system reset
        -- i_begin          - goes to logic 1 for 1 clock cycle as signal to start calculation
        -- i_amplitude      - amplitude of sine/cosine signal (radius of circle in calculation)
        -- i_angle          - angle in which sin()and cos() are evaluated, range 0° -> +90° (X"00000000" -> X"40000000")
        -- o_done           - is 0 if calculation is ongoing, 1 if calculation is done
        -- o_sin            - result of sine()
        -- o_cos            - result of cos()
        
        i_clk               : in    std_logic;
        i_rst               : in    std_logic;
        i_begin             : in    std_logic;
        i_amplitude         : in    unsigned(31 downto 0);
        i_angle             : in    unsigned(31 downto 0);      -- 32 bit (0° -> +90°)
        
        o_done              : out   std_logic;
        o_sin               : out   unsigned(63 downto 0);
        o_cos               : out   unsigned(63 downto 0)
        );
end CORDIC;

architecture Behavioral of CORDIC is

    -- C_K                  - CORDIC scaling factor, equals to 0.607259350088812561694 (32-bit fixed point, 30 bits fraction)
    
    constant    C_K                 : unsigned(31 downto 0)     := X"26DD3B6A";
    
    -- r_CORDIC_state       - state of CORDIC module
    -- r_angle              - angle in which CORDIC calculates sin() and cos()
    -- r_angle_calc         - approximated angle during iterations
    -- r_x                  - x-coordinate on the circle
    -- r_y                  - y-coordinate on the circle
    -- r_i                  - index of iteration
    
    type        t_CORDIC_state is (t_IDLE, t_CALCULATING, t_RESULT);
    signal      r_CORDIC_state         : t_CORDIC_state := t_IDLE;
    
    signal      r_angle             : signed(31 downto 0)           := X"00000000";
    signal      r_angle_calc        : signed(31 downto 0)           := X"00000000";
    signal      r_x                 : signed(31 downto 0)           := X"00000000";
    signal      r_y                 : signed(31 downto 0)           := X"00000000";
    signal      r_i                 : integer range 0 to 32         := 0;
    
    -- CORDIC iteration angles and tangents of the angles:
    -- INDEX        ANGLE (°)            32-bit number (X"FFFFFFFF" = 360°)
    -- 00           45.0000000           20000000
    -- 01           26.5650512           12E4051E
    -- 02           14.0362435           09FB385B
    -- 03           7.12501635           051111D4
    -- 04           3.57633437           028B0D43
    -- 05           1.78991061           0145D7E1
    -- 06           0.89517371           00A2F61E
    -- 07           0.44761417           00517C55
    -- 08           0.22381050           0028BE53
    -- 09           0.11190568           00145F2F
    -- 10           0.05595289           000A2F98
    -- 11           0.02797645           000517CC
    -- 12           0.01398823           00028BE6
    -- 13           0.00699411           000145F3
    -- 14           0.00349706           0000A2FA
    -- 15           0.00174853           0000517D
    -- 16           0.00087426           000028BE
    -- 17           0.00043713           0000145F
    -- 18           0.00021857           00000A30
    -- 19           0.00010928           00000518
    -- 20           0.00005464           0000028C
    -- 21           0.00002732           00000146
    -- 22           0.00001366           000000A3
    -- 23           0.00000683           00000051
    -- 24           0.00000342           00000029
    -- 25           0.00000171           00000014
    -- 26           0.00000085           0000000A
    -- 27           0.00000043           00000005
    -- 28           0.00000021           00000003
    -- 29           0.00000011           00000001
    -- 30           0.00000005           00000001
    -- 31           0.00000003           00000000

begin

    -- process p_CORDIC is implementation of Coordinate Rotation Digital Computer
    -- CORDIC calculates values of sin() and cos() of given angle using iterations
    -- sin() and cos() are approximated as x and y coordinates on circle x = cos(), y = sin()
    -- by adding and subtracting specified values of angles, we can approximate given angle after a number of iterations
    -- any x and y coordinates are calculated by using values of x and y, tan() and cos() from previous iteration
    -- CORDIC does the calculation very fast because of replacing "long operations" as multiplication by shifting bits
    -- using iteration angles where tan() is equal to 1/2, 1/4, 1/8, 1/16, etc. replaces multiplication by bit shifting
    -- final step of algorithm is multiplication of results by scaling constant of CORDIC K = 0.607259350088812561694
    p_CORDIC : process(i_clk)
    begin
        if (rising_edge(i_clk)) then
            if (i_rst = '1') then
                r_x <= (others => '0');
                r_y <= (others => '0');
                r_angle_calc <= (others => '0');
                r_i <= 0;
                o_done <= '0';
                r_CORDIC_state <= t_IDLE;
            else
                case r_CORDIC_state is
                    -- =========================================================
                    -- wait for i_begin pulse and set starting angle and x value
                    when t_IDLE =>
                        if (i_begin = '1') then
                            r_CORDIC_state <= t_CALCULATING;
                            r_i <= 0;
                            r_x <= signed(i_amplitude);
                            r_y <= (others => '0');
                            r_angle <= signed(i_angle);
                            r_angle_calc <= (others => '0');
                            o_done <= '0';
                        end if;
                    
                    -- ============================================================
                    -- iterate using angle values and calculate x and y coordinates
                    when t_CALCULATING =>
                        -- if current iteration of angle is lower than given angle, add next value
                        if (r_angle_calc <= r_angle) then
                            if (r_i = 0) then
                                r_angle_calc <= r_angle_calc + X"20000000";
                            elsif (r_i = 1) then
                                r_angle_calc <= r_angle_calc + X"12E4051E";
                            elsif (r_i = 2) then
                                r_angle_calc <= r_angle_calc + X"09FB385B";
                            elsif (r_i = 3) then
                                r_angle_calc <= r_angle_calc + X"051111D4";
                            elsif (r_i = 4) then
                                r_angle_calc <= r_angle_calc + X"028B0D43";
                            elsif (r_i = 5) then
                                r_angle_calc <= r_angle_calc + X"0145D7E1";
                            elsif (r_i = 6) then
                                r_angle_calc <= r_angle_calc + X"00A2F61E";
                            elsif (r_i = 7) then
                                r_angle_calc <= r_angle_calc + X"00517C55";
                            elsif (r_i = 8) then
                                r_angle_calc <= r_angle_calc + X"0028BE53";
                            elsif (r_i = 9) then
                                r_angle_calc <= r_angle_calc + X"00145F2F";
                            elsif (r_i = 10) then
                                r_angle_calc <= r_angle_calc + X"000A2F98";
                            elsif (r_i = 11) then
                                r_angle_calc <= r_angle_calc + X"000517CC";
                            elsif (r_i = 12) then
                                r_angle_calc <= r_angle_calc + X"00028BE6";
                            elsif (r_i = 13) then
                                r_angle_calc <= r_angle_calc + X"000145F3";
                            elsif (r_i = 14) then
                                r_angle_calc <= r_angle_calc + X"0000A2FA";
                            elsif (r_i = 15) then
                                r_angle_calc <= r_angle_calc + X"0000517D";
                            elsif (r_i = 16) then
                                r_angle_calc <= r_angle_calc + X"000028BE";
                            elsif (r_i = 17) then
                                r_angle_calc <= r_angle_calc + X"0000145F";
                            elsif (r_i = 18) then
                                r_angle_calc <= r_angle_calc + X"00000A30";
                            elsif (r_i = 19) then
                                r_angle_calc <= r_angle_calc + X"00000518";
                            elsif (r_i = 20) then
                                r_angle_calc <= r_angle_calc + X"0000028C";
                            elsif (r_i = 21) then
                                r_angle_calc <= r_angle_calc + X"00000146";
                            elsif (r_i = 22) then
                                r_angle_calc <= r_angle_calc + X"000000A3";
                            elsif (r_i = 23) then
                                r_angle_calc <= r_angle_calc + X"00000051";
                            elsif (r_i = 24) then
                                r_angle_calc <= r_angle_calc + X"00000029";
                            elsif (r_i = 25) then
                                r_angle_calc <= r_angle_calc + X"00000014";
                            elsif (r_i = 26) then
                                r_angle_calc <= r_angle_calc + X"0000000A";
                            elsif (r_i = 27) then
                                r_angle_calc <= r_angle_calc + X"00000005";
                            elsif (r_i = 28) then
                                r_angle_calc <= r_angle_calc + X"00000003";
                            elsif (r_i = 29) then
                                r_angle_calc <= r_angle_calc + X"00000001";
                            elsif (r_i = 30) then
                                r_angle_calc <= r_angle_calc + X"00000001";
                                r_CORDIC_state <= t_RESULT;
                            end if;
                            -- calculate x and y coordinates on the circle
                            r_x <= r_x - shift_right(r_y, r_i);
                            r_y <= r_y + shift_right(r_x, r_i);
                        -- if current iteration of angle is higher than given angle, subtract next value
                        else
                            if (r_i = 0) then
                                r_angle_calc <= r_angle_calc - X"20000000";
                            elsif (r_i = 1) then
                                r_angle_calc <= r_angle_calc - X"12E4051E";
                            elsif (r_i = 2) then
                                r_angle_calc <= r_angle_calc - X"09FB385B";
                            elsif (r_i = 3) then
                                r_angle_calc <= r_angle_calc - X"051111D4";
                            elsif (r_i = 4) then
                                r_angle_calc <= r_angle_calc - X"028B0D43";
                            elsif (r_i = 5) then
                                r_angle_calc <= r_angle_calc - X"0145D7E1";
                            elsif (r_i = 6) then
                                r_angle_calc <= r_angle_calc - X"00A2F61E";
                            elsif (r_i = 7) then
                                r_angle_calc <= r_angle_calc - X"00517C55";
                            elsif (r_i = 8) then
                                r_angle_calc <= r_angle_calc - X"0028BE53";
                            elsif (r_i = 9) then
                                r_angle_calc <= r_angle_calc - X"00145F2F";
                            elsif (r_i = 10) then
                                r_angle_calc <= r_angle_calc - X"000A2F98";
                            elsif (r_i = 11) then
                                r_angle_calc <= r_angle_calc - X"000517CC";
                            elsif (r_i = 12) then
                                r_angle_calc <= r_angle_calc - X"00028BE6";
                            elsif (r_i = 13) then
                                r_angle_calc <= r_angle_calc - X"000145F3";
                            elsif (r_i = 14) then
                                r_angle_calc <= r_angle_calc - X"0000A2FA";
                            elsif (r_i = 15) then
                                r_angle_calc <= r_angle_calc - X"0000517D";
                            elsif (r_i = 16) then
                                r_angle_calc <= r_angle_calc - X"000028BE";
                            elsif (r_i = 17) then
                                r_angle_calc <= r_angle_calc - X"0000145F";
                            elsif (r_i = 18) then
                                r_angle_calc <= r_angle_calc - X"00000A30";
                            elsif (r_i = 19) then
                                r_angle_calc <= r_angle_calc - X"00000518";
                            elsif (r_i = 20) then
                                r_angle_calc <= r_angle_calc - X"0000028C";
                            elsif (r_i = 21) then
                                r_angle_calc <= r_angle_calc - X"00000146";
                            elsif (r_i = 22) then
                                r_angle_calc <= r_angle_calc - X"000000A3";
                            elsif (r_i = 23) then
                                r_angle_calc <= r_angle_calc - X"00000051";
                            elsif (r_i = 24) then
                                r_angle_calc <= r_angle_calc - X"00000029";
                            elsif (r_i = 25) then
                                r_angle_calc <= r_angle_calc - X"00000014";
                            elsif (r_i = 26) then
                                r_angle_calc <= r_angle_calc - X"0000000A";
                            elsif (r_i = 27) then
                                r_angle_calc <= r_angle_calc - X"00000005";
                            elsif (r_i = 28) then
                                r_angle_calc <= r_angle_calc - X"00000003";
                            elsif (r_i = 29) then
                                r_angle_calc <= r_angle_calc - X"00000001";
                            elsif (r_i = 30) then
                                r_angle_calc <= r_angle_calc - X"00000001";
                                r_CORDIC_state <= t_RESULT;
                            end if;
                            -- calculate x and y coordinates on the circle
                            r_x <= r_x + shift_right(r_y, r_i);
                            r_y <= r_y - shift_right(r_x, r_i);
                        end if;
                            
                        r_i <= r_i + 1;     -- increment r_i after every iteration
                        
                    -- =========================================================
                    -- calculate final value of sine and cosine using constant K
                    when t_RESULT =>
                        o_cos <= unsigned(r_x) * C_K;
                        o_sin <= unsigned(r_y) * C_K;
                        o_done <= '1';
                        r_CORDIC_state <= t_IDLE;
                        
                    when others =>
                        
                end case;
            end if;
        end if;
    end process;

end Behavioral;
