#ifndef _TICKABLE_H_
#define _TICKABLE_H_

class i_tickable
{
public:

	virtual void tick(float delta_time) = 0;

private:

};


#endif // _TICKABLE_H_
