#include "Pigeon/ECS/System.h"

namespace sbx
{
	class SampleUISystem : public pig::System
	{
	public:
		SampleUISystem() = default;
		~SampleUISystem() = default;
		void Update(const pig::Timestep& ts) override;
		pig::SystemAccessDecl DeclareAccess() const override;
	private:
	};
}
