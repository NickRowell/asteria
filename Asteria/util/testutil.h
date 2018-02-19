#ifndef TESTUTIL_H
#define TESTUTIL_H

class TestUtil
{
public:
    TestUtil();

    static void testRandomVector();

    static void testLevenbergMarquardtFitter();

    static void testLevenbergMarquardtFitterCovariance();

    static void testRaDecAzElConversion();

    static void testImagedReadWrite();

};

#endif // TESTUTIL_H
