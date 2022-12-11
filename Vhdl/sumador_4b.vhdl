library ieee;
use ieee.std_logic_1164.all;

entity sumador_4b is
  Port (clk_port: in std_logic;
        SC: in std_logic;
        SM: out std_logic;
        input_a: in std_logic;
        input_b: in std_logic);
end entity sumador_4b;

architecture estructural_s4b of sumador_4b is
  
  component SR
    port ( Si: in std_logic;
          clock: in std_logic;
          So: out std_logic);
  end component;
  
  component biestable
    port( D: in std_logic;
          clock: in std_logic;
          Q: out std_logic);
  end component;
  
  component sumador
     port( A: in std_logic;
          B: in std_logic;
          C_in: in std_logic;
          C_out: out std_logic;
          S: out std_logic);
  end component;

  signal clk_signal: std_logic;
  signal i_0: std_logic := '0';
  signal i_1: std_logic := '0';
  signal i_2: std_logic := '0';
  signal carry: std_logic;
  signal s_out: std_logic;
  signal output: std_logic;

begin
  SR_A : SR
    port map (
      Si => input_a,
      clock => clk_signal,
      So => i_0);
  SR_B : SR
    port map (
      Si => input_b,
      clock => clk_signal,
      So => i_1);
  
  SR_C : SR
    port map (
      Si => s_out,
      clock => clk_signal,
      So => output);

  FULL_ADDER : sumador
    port map (
      A => i_0,
      B => i_1,
      C_in => i_2,
      C_out => carry,
      S => s_out);

  BIESTABLE_C : biestable
    port map (
      D => carry,
      clock => clk_signal,
      Q => i_2);

  clk_signal <= clk_port nand SC;
  SM <= output;

 end estructural_s4b;
