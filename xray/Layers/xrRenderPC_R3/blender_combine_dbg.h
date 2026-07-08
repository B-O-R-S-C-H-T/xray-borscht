#pragma once

class CBlender_combine_dbg : public IBlender  
{
public:
	virtual		LPCSTR		getComment()	{ return "INTERNAL: G-buffer debug";	}
	virtual		BOOL		canBeDetailed()	{ return FALSE;	}
	virtual		BOOL		canBeLMAPped()	{ return FALSE;	}

	virtual		void		Compile			(CBlender_Compile& C);

	CBlender_combine_dbg();
	virtual ~CBlender_combine_dbg();
};


