#ifndef IM_ManipulatorH
#define IM_ManipulatorH

class IM_Manipulator {
public:
	bool m_active;

	IM_Manipulator() : m_active(false) {}

	void Render();

private:
	bool				 m_moving	= false;
	bool				 m_scaling	= false;
	bool				 m_rotating = false;
	std::vector<Fvector> m_oldPositionValues;
	std::vector<Fvector> m_oldScaleValues;
	std::vector<Fvector> m_oldRotateValues;
	Fvector				 m_oldCamPos;
	Fvector				 m_oldCamHPB;
};

extern IM_Manipulator imManipulator;

#endif