#include <fstream>
#include <time.h>

#include "Locations.h"
#include "Miner.h"
#include "MinersWife.h"
#include "Swain.h"
#include "BarFly.h"
#include "EntityManager.h"
#include "MessageDispatcher.h"
#include "misc/ConsoleUtils.h"
#include "EntityNames.h"
#include "GUI.h"

std::ofstream os;

int main()
{

//define this to send output to a text file (see locations.h)
#ifdef TEXTOUTPUT
  os.open("output.txt");
#endif

  //seed random number generator
  srand((unsigned) time(NULL));

  //create a miner
  Miner* Bob = new Miner(ent_Miner_Bob);

  //create his wife
  MinersWife* Elsa = new MinersWife(ent_Elsa);

  //create a swain
  Swain* JeanErnestain = new Swain(ent_Swain);

  //create a BarFly
  BarFly* Bernard = new BarFly(ent_BarFly);

  //register them with the entity manager
  EntityMgr->RegisterEntity(Bob);
  EntityMgr->RegisterEntity(Elsa);
  EntityMgr->RegisterEntity(JeanErnestain);
  EntityMgr->RegisterEntity(Bernard);
  
  //Create one thread for each entity
  sf::Thread BobThread(&Miner::HandleThread, Bob);
  sf::Thread ElsaThread(&MinersWife::HandleThread, Elsa);
  sf::Thread JeanThread(&Swain::HandleThread, JeanErnestain);
  sf::Thread BernardThread(&BarFly::HandleThread, Bernard);

  //Launch the threads
  BobThread.launch();
  ElsaThread.launch();
  JeanThread.launch();
  BernardThread.launch();
  
  //Take care of the messages
  for (int i=0; i<30; ++i)
  { 
    Dispatch->DispatchDelayedMessages();
    Sleep(800);
  }

  //Wait for all of the threads to finish
  BobThread.wait();
  ElsaThread.wait();
  JeanThread.wait();
  BernardThread.wait();

  //tidy up
  delete Bob;
  delete Elsa;
  delete JeanErnestain;
  delete Bernard;

  //wait for a keypress before exiting
  PressAnyKeyToContinue();
 
  return 0;
}






