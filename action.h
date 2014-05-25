#ifndef ACTION_H
#define ACTION_H
#include "defs.h"
#include "cstdlib"
#include "ctime"
#include "trafgen.h"
#include "node.h"
#include "router.h"
#include "time_manager.h"

using namespace std;
extern router* router0;
extern vector<trafgen *> t;
extern vector<node *> n;
extern time_manager* tM;
extern int nodes;
void initi();

void next_step();
#endif // ACTION_H
