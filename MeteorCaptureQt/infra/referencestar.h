#ifndef REFERENCESTAR_H
#define REFERENCESTAR_H


class ReferenceStar
{
public:
    ReferenceStar(double ra, double dec, double mag);

    /**
     * @brief ra
     * The (fixed) Right Ascension of the reference star [radians]
     */
    const double ra;

    /**
     * @brief dec
     * The (fixed) Declination of the reference star [radians]
     */
    const double dec;

    /**
     * @brief vt
     * The (fixed) apparent magnitude of the reference star [mag]
     */
    const double mag;






};

#endif // REFERENCESTAR_H
