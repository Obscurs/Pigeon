#include "Pigeon/ECS/System.h"

namespace sbx
{
	class SampleUISystem : public pg::System
	{
	public:
		SampleUISystem() = default;
		~SampleUISystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	private:
	};
}
