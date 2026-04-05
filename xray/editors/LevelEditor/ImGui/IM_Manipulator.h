#ifndef IM_ManipulatorH
#define IM_ManipulatorH

class IM_Manipulator {
public:
	bool m_active;

	IM_Manipulator() : m_active(false), m_moving(false), m_scaling(false), m_rotating(false), m_oldCamPos(Fvector()), m_oldCamHPB(Fvector()) {}

	void Render();

private:
	bool				 m_moving;
	bool				 m_scaling;
	bool				 m_rotating;
	std::vector<Fvector> m_oldPositionValues;
	std::vector<Fvector> m_oldScaleValues;
	std::vector<Fvector> m_oldRotateValues;
	Fvector				 m_oldCamPos;
	Fvector				 m_oldCamHPB;
};

extern IM_Manipulator imManipulator;

#endif
