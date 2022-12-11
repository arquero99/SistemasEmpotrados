library ieee;
use ieee.std_logic_1164.all;

entity biestable is 
    port( D: in std_logic;
          clock: in std_logic;
          Q: out std_logic);
          
end entity biestable;


architecture behaviour_biestable of biestable is
    signal registro: std_logic := '0';
begin 
        process (clock)
        begin 
            if(clock'event and clock='1') then 
                Q <= registro;
                registro <= D;
            end if;
        end process;
    
end architecture behaviour_biestable;
