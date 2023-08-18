#include "plugin.h"
#include "Rubbish.h"
#include <filesystem>
#include "O:/VCSPC-master3/VCSPC-master/VCS PC/debugmenu_public.h"
using namespace plugin;
DebugMenuAPI gDebugMenuAPI;
namespace fs = std::filesystem;


class RubbishSA {
public:
    RubbishSA() {
        plugin::Events::initRwEvent += []() {
            CRubbish::Init();
        };

        plugin::Events::gameProcessEvent += []() {
            CRubbish::Update();
        };
        plugin::CdeclEvent <plugin::AddressList<0x53EAD3, plugin::H_CALL>, plugin::PRIORITY_AFTER, plugin::ArgPickNone, void()> onRenderEffects;
        onRenderEffects += []() {
            CRubbish::Render();
        };
        plugin::Events::shutdownRwEvent += []() {
            CRubbish::Shutdown();
        };
        if (DebugMenuLoad()) {
            DebugMenuAddVarBool8("Rendering", "Rubbish invisible", (int8_t*)&CRubbish::bRubbishInvisible, NULL);
        }
        plugin::ThiscallEvent <plugin::AddressList<0x6B19F9, plugin::H_CALL>, plugin::PRIORITY_AFTER, plugin::ArgPickN<CAutomobile*, 0>, void(CAutomobile*)> onProcessControl;
        onProcessControl += [](CAutomobile* _this) {
            CRubbish::StirUp(_this);
        };
            if (fs::exists("MODELS\\RUBBISHSA.TXD")) {
                // The file exists, do nothing
            }
            else {
                MessageBox(HWND_DESKTOP, "RubbishSA.txd cannot be found in models folder. If you are 100% sure that file exists, then install .asi in root game folder, instead of scripts folder. The game will continue to load, but will crash on loading screen.", "rubbishSA.asi", MB_ICONERROR);
            }
    }
} rubbishSA;
