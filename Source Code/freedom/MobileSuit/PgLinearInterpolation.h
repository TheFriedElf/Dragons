#ifndef FREEDOM_DRAGONICA_UTIL_PGLINEARINTERPLATION_H
#define FREEDOM_DRAGONICA_UTIL_PGLINEARINTERPLATION_H

class PgLinearInterpolation
{
public:
	PgLinearInterpolation():m_fValue(0.f),m_fStep(0.f),m_fRemainingTime(0.f) {}

    bool Setup(float const from, float const to, float const time)
    {
        if(time < 0)
        {
            return false;
        }
        m_fRemainingTime = time;
        m_fValue = from;
        m_fStep = (to-from)/time; // Calculate distance per second.
        return true;
    }
    // Return TRUE when the target has been reached or passed.
    bool Interpolate(float const deltaTime)
    {
        m_fRemainingTime -= deltaTime;
        m_fValue += m_fStep*deltaTime;
        return !IsUse();
    }
	bool IsUse()const
	{
		return (m_fRemainingTime > 0);
	}
    float GetValue()const
    {
        return m_fValue;
    }
private:
    float m_fValue;
    float m_fStep;
    float m_fRemainingTime;
};
#endif // FREEDOM_DRAGONICA_UTIL_PGLINEARINTERPLATION_H