#include <vector>
#include <iostream>
#include <cmath>
#include "gem_utils_managem.hpp"
#include "gemset_multithreaded.hpp"
#include "managem_limiter.hpp"
using namespace std;

const int MANAGEM_VALUE=512;
const int AMP_MAX_VALUE=128;
const float GROWTH = 0.6343;

bool better_orange (const Gem* a, const Gem* b)
{
  return a->orange>b->orange;
}

float amp_managem_power (const Gem* managem, const Gem* amplifier)
{
  int value = managem->value+6*amplifier->value;
  float power = (managem->orange*1.5+6*0.23*2.8*amplifier->orange)*managem->black;
  return power/pow(value, GROWTH);
}

int main ()
{
  vector<Gem*>* basegems_managem = new vector<Gem*>;
  basegems_managem->push_back(new Gem(0, 1, 0));
  basegems_managem->push_back(new Gem(0, 0, 1));
  vector<Gem*>** gemset_managem=generate_gemset_multithreaded(basegems_managem, MANAGEM_VALUE, limit_managem);

  vector<Gem*>* basegems_amplifier = new vector<Gem*>;
  basegems_amplifier->push_back(new Gem(0, 1, 0));
  vector<Gem*>** gemset_amplifier=generate_gemset_multithreaded(basegems_amplifier, AMP_MAX_VALUE, limit_managem);

  vector<Gem*>* managems = gemset_managem[MANAGEM_VALUE];
  
  vector<Gem*>* amplifiers = new vector<Gem*>;
  for (int i=1; i<=AMP_MAX_VALUE; i++)
    amplifiers->push_back(best_from(gemset_amplifier[i], better_orange));
  
  Gem* best_managem = (*managems)[0];
  Gem* best_amplifier = (*amplifiers)[0];
  float power = amp_managem_power(best_managem, best_amplifier);
  for (Gem* m : *managems)
    for (Gem* a : *amplifiers)
      if (amp_managem_power(m, a)>power)
      {
        power=amp_managem_power(m, a);
        best_managem=m;
        best_amplifier=a;
      }
  print_tree(best_managem);
  print_tree(best_amplifier);
  print_stats(best_managem);
  print_stats(best_amplifier);
  print_equations(best_managem);
  return 0;
}
