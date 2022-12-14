library ieee;
use ieee.std_logic_1164.all;
entity main is
end main;

architecture behaviour of main is

  component sumador_4b
     port (clk_port: in std_logic;
        SC: in std_logic;
        SM: out std_logic;
        input_a: in std_logic;
        input_b: in std_logic);
  end component;

  signal in_a : std_logic := '0';
  signal in_b : std_logic := '0';
  signal input_sc : std_logic := '1';
  signal input_clk : std_logic := '0';
  signal output : std_logic := '0';

begin
  sumador : sumador_4b
    Port map (
      clk_port => input_clk,
      SC => input_sc,
      input_a => in_a,
      input_b => in_b,
      SM => output);

  gen_clk : process
  begin
    input_clk <= not input_clk;
    wait for 5 ns;
  end process;

  main_proc : process
  begin
    loop
      in_a <= '1';
      in_b <= '0';
      input_sc <= '1';
      wait for 10 ns;

      in_a <= '0';
      in_b <= '1';
      input_sc <= '1';
      wait for 10 ns;

      in_a <= '1';
      in_b <= '1';
      input_sc <= '1';
      wait for 10 ns;

      in_a <= '0';
      in_b <= '0';
      input_sc <= '1';
      wait for 10 ns;

    end loop;
  end process;                             
end behaviour;
