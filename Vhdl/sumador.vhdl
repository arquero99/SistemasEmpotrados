library ieee;
use ieee.std_logic_1164.all;

entity sumador is 
    port( A: in std_logic;
          B: in std_logic;
          C_in: in std_logic;
          C_out: out std_logic;
          S: out std_logic);
          
end entity sumador;


architecture behaviour_sumador of sumador is
    signal AB: std_logic;
    signal AC: std_logic;
    signal BC: std_logic;
begin 
   S <= A xor B xor C_in;
   
   AB <= A and B;
   AC <= A and C_in;
   BC <= B and C_in;
   
   C_out <= AB or AC or BC;
end architecture;
