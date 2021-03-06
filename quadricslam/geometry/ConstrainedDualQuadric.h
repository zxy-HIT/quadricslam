/* ----------------------------------------------------------------------------

 * QuadricSLAM Copyright 2020, ARC Centre of Excellence for Robotic Vision, Queensland University of Technology (QUT)
 * Brisbane, QLD 4000
 * All Rights Reserved
 * Authors: Lachlan Nicholson, et al. (see THANKS for the full author list)
 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file ConstrainedDualQuadric.h
 * @date Apr 14, 2020
 * @author Lachlan Nicholson
 * @brief a constrained dual quadric
 */

#pragma once

#include <quadricslam/geometry/AlignedBox3.h>

#include <gtsam/geometry/Pose3.h>
#include <gtsam/nonlinear/Values.h>
#include <gtsam/geometry/Cal3_S2.h>

#include <random>

namespace gtsam {

  /**
   * @class ConstrainedDualQuadric
   * A constrained dual quadric (r,t,s): see Nicholson et al. 2019 for details
   */
  class ConstrainedDualQuadric {

    protected:
      Pose3 pose_; ///< 3D pose of ellipsoid
      Vector3 radii_; ///< radii of x,y,z axii

    public:
      EIGEN_MAKE_ALIGNED_OPERATOR_NEW

      /// @name Constructors and named constructors
      /// @{
      
      /** default constructor, unit sphere at origin */
      ConstrainedDualQuadric();

      /**
       * Constructor from 4x4 Matrix,
       * Here we ensure the quadric is ellipsoidal
       * and constrain manually if it is not. 
       * @param dQ
       */
      ConstrainedDualQuadric(const Matrix44& dQ);

      /**
       * Constructor pose and radii
       * @param pose quadric pose (Pose3)
       * @param radii quadric radii (Vector3)
       */
      ConstrainedDualQuadric(const Pose3& pose, const Vector3& radii) : 
        pose_(pose), radii_(radii) {};

      /**
       * Constructor from rotation, translation and shape
       * @param R quadric rotation (Rot3)
       * @param t quadric translation (Point3)
       * @param r quadric radii (Vector3)
       */
      ConstrainedDualQuadric(const Rot3& R, const Point3& t, const Vector3& r) :
        pose_(Pose3(R,t)), radii_(r) {};

      /** 
       * Constrains a generic dual quadric surface to be ellipsoidal
       * @param dual_quadric 4x4 symmetric matrix (Matrix4)
      */
      static ConstrainedDualQuadric constrain(const Matrix4& dual_quadric);

      /// @}
      /// @name Class accessors
      /// @{

      /** Get pose, avoid computation with it */
      Pose3 pose(void) const {return pose_;}

      /** Get quadric radii, avoid computation with it */
      Vector3 radii(void) const {return radii_;}

      /** Get quadric centroid */
      Point3 centroid(void) const {return pose_.translation();}

      /// @}
      /// @name Class methods
      /// @{

      /**
       * Constructs 4x4 quadric matrix from pose & radii
       * Q = Z * Qc * Z.T
       * Z = quadric pose in global frame
       * Qc = centered dualquadric diagonal matrix of shape (s1^2, s2^2, s3^2, -1)
       * where s1,s2,s3 are the radius of each axis on the ellipse
       * see Nicholson et. al 2019 QuadricSLAM for full details
       * @return 4x4 constrained quadric
       */
      Matrix44 matrix(OptionalJacobian<16,9> H = boost::none) const;

      /** Returns the normalized dual quadric in matrix form */
      Matrix44 normalizedMatrix(void) const;

      /**
       * Calculates the AlignedBox3 bounds of the ellipsoid
       * @return 3D axis aligned bounding box
       */ 
      AlignedBox3 bounds() const;

      /** Returns true if quadric centroid has negative depth */
      bool isBehind(const Pose3& cameraPose) const;

      /** 
       * Returns true if quadric contains point 
       * Points on the edge of the quadric are considered contained
       */
      bool contains(const Pose3& cameraPose) const;

      /// @}
      /// @name Manifold group traits
      /// @{
      enum { dimension = 9 };

      /** The Retract at origin */
      static ConstrainedDualQuadric Retract(const Vector9& v);

      /** The Local at origin */
      static Vector9 LocalCoordinates(const ConstrainedDualQuadric& q);

      /**
       * Moves from this by v in tangent space, then retracts back to a quadric
       * @param v displacement vector in tangent space
       * @return ConstrainedDualQuadric on the manifold 
       */
      ConstrainedDualQuadric retract(const Vector9& v) const;

      /**
       * Calculates the distance in tanget space between two quadrics on the manifold
       * @param other another ConstrainedDualQuadric
       * @return vector between ellipsoids in tangent space
       */
      Vector9 localCoordinates(const ConstrainedDualQuadric& other) const;

      /** Add quadric to values */
      void addToValues(Values &v, const Key& k);

      /** Get Quadric from values */
      static ConstrainedDualQuadric getFromValues(const Values &v, const Key& k);

      /// @}
      /// @name Testable group traits
      /// @{
        
      /** Prints the dual quadric with optional string */
      void print(const std::string& s = "") const;

      /** Compares two ConstrainedDualQuadrics */
      /// TODO: account for scaling by normalizing quadric
      bool equals(const ConstrainedDualQuadric& other, double tol = 1e-9) const;

      /// @}
  };

  // Add ConstrainedDualQuadric to Manifold group
  template <>
  struct traits<ConstrainedDualQuadric> : public internal::Manifold<ConstrainedDualQuadric> {};

  template <>
  struct traits<const ConstrainedDualQuadric> : public internal::Manifold<ConstrainedDualQuadric> {};

} // namespace gtsam
