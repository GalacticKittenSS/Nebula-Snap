#include <Nebula.h>
#include <Nebula_EntryPoint.h>

#include "Snap.h"

namespace Nebula {
	class GenericApp : public Application {
	public:
		GenericApp(ApplicationCommandLineArgs args): Application("Snap", args) {
			PushLayer(new Snap());
		}

		~GenericApp() { }
	};

	Application* CreateApplication(ApplicationCommandLineArgs args) {
		return new GenericApp(args);
	}
}