#include <Nebula.h>
#include <Nebula_EntryPoint.h>

#include "Snap.h"

class GenericApp : public Nebula::Application {
public:
	GenericApp(Nebula::ApplicationCommandLineArgs args): Application("Snap", args) {
		PushLayer(new Snap());
	}

	~GenericApp() { }
};

namespace Nebula {
	Application* CreateApplication(ApplicationCommandLineArgs args) {
		return new GenericApp(args);
	}
}