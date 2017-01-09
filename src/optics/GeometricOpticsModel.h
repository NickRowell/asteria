#ifndef GEOMETRICOPTICSMODEL_H_
#define GEOMETRICOPTICSMODEL_H_

/**
 * The GeometricOpticsModel class provides a base for all models of the camera
 * projective geometry. The main operations required are projecting vectors expressed
 * in the 3D camera frame to a point in the image, and deprojecting points in the
 * image to a 3D vector in the camera frame.
 */
template < typename T >
class GeometricOpticsModel {
public:

	GeometricOpticsModel();

	virtual ~GeometricOpticsModel();

	/**
	 * Get the 1D vector of parameters of the geometry model.
	 * \param Pointer to an int; on exit, this contains the number of
	 * parameters stored in the array pointed to by the returned value.
	 * \return Pointer to the start of the array of double values containing
	 * the parameters of the geometric model.
	 */
	virtual double * getParameters(int &);

	/**
	 * Sets the parameters of the geometric model of the projective optics.
	 * \param Pointer to the start of the array of double values containing
	 * the parameters of the geometric model.
	 */
	virtual void setParameters(double *);

	/**
	 * Get the unit vector towards a given detector pixel
	 * coordinate, correcting for any radial distortion.
	 * \param ip   i coordinate of pixel in distorted image.
	 * \param jp   j coordinate of pixel in distorted image.
	 * \return     Camera frame unit vector towards the pixel.
	 */
	virtual Vec3<T> deprojectPixel(const T &, const T &) const;

	/**
	 * Project the given camera frame position vector into the image plane,
	 * applying forwards radial distortion.
	 * \param X_vec     Camera frame position vector.
	 * \param ip        On exit, contains the i image coordinates.
	 * \param jp        On exit, contains the j image coordinates.
	 */
	virtual void projectVector(const Vec3<T> & X_cam, T & ip, T & jp) const;

};

#endif /* GEOMETRICOPTICSMODEL_H_ */
