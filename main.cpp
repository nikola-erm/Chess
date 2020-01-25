#include <ai/binsearch_nocache_player.h>
#include <ai/human_player.h>
#include <engine/engine.h>

#include <memory>

using namespace std;

int main() {
    NEngine::TEngine engine;
    unique_ptr<NEngine::IPlayer> whites(new NAI::THumanPlayer());
    unique_ptr<NEngine::IPlayer> blacks(new NAI::TBinSearchNoCachePlayer(1e9));
    engine.Play(whites.get(), blacks.get());    
}