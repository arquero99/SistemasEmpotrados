library ieee;
use ieee.std_logic_1164.all;

entity SR is 
    port( Si: in std_logic;
          clock: in std_logic;
          So: out std_logic);
          
end entity SR;


architecture behaviour_SR of SR is
    signal dq_array: std_logic_vector (3 downto 0);
begin 
        process (clock)
        begin 
            if(clock'event and clock='1') then 
                dq_array <= Si & dq_array(3 downto 1);
            end if;
        end process;
    So <= dq_array(0);
end architecture;
