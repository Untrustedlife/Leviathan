#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Application.h"

namespace Leviathan{

    class ServerApplication : public LeviathanApplication{
    public:
        DLLEXPORT ServerApplication();
        DLLEXPORT ~ServerApplication();

        //! Makes sure doesn't start in GUI mode
        DLLEXPORT bool PassCommandLine(int argcount, char* args[]) override;

        NETWORKED_TYPE GetProgramNetType() const override {
            
            return NETWORKED_TYPE::Server;
        }


    protected:

    };

}

