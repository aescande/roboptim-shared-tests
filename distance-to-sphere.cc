// Copyright (C) 2011-2013 by Florent Lamiraux, Thomas Moulard, AIST, CNRS.
//
// This file is part of the roboptim.
//
// roboptim is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// roboptim is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with roboptim.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>

#include <boost/mpl/list.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/test/unit_test.hpp>

#include <log4cxx/basicconfigurator.h>

#include <roboptim/core/differentiable-function.hh>
#include <roboptim/core/io.hh>
#include <roboptim/core/optimization-logger.hh>
#include <roboptim/core/solver.hh>
#include <roboptim/core/solver-factory.hh>
#include <roboptim/core/sum-of-c1-squares.hh>

#ifndef SOLVER_NAME
# error "please define solver name"
#endif //! PROBLEM_TYPE

#ifndef PLUGIN_PATH
# error "please define plug-in path"
#endif //! PROBLEM_TYPE

#define FORWARD_TYPEDEFS()                              \
  typedef GenericDifferentiableFunction<T> parent_t;    \
  typedef typename parent_t::result_t result_t;         \
  typedef typename parent_t::size_type size_type;       \
  typedef typename parent_t::argument_t argument_t;     \
  typedef typename parent_t::gradient_t gradient_t;     \
  typedef typename parent_t::jacobian_t jacobian_t

typedef boost::mpl::list< ::roboptim::EigenMatrixDense> functionTypes_t;

struct TestSuiteConfiguration
{
  TestSuiteConfiguration ()
  {
    log4cxx::BasicConfigurator::configure ();

    lt_dlinit();
    BOOST_REQUIRE_EQUAL (lt_dlsetsearchpath (PLUGIN_PATH), 0);
  }

  ~TestSuiteConfiguration ()
  {
    lt_dlexit ();
  }
};

namespace roboptim
{
  namespace distanceToSphere
  {
    struct ExpectedResult
    {
      static const double x0[];
      static const double fx0;
      static const double x[];
      static const double fx;
    };
    const double ExpectedResult::x0[] = {0., 0.};
    const double ExpectedResult::fx0  = 4.8974713057829096;
    const double ExpectedResult::x[]  = {-1.5, -1.2};
    const double ExpectedResult::fx   = 1.0;

    /// Distance between a point on unit sphere and another point in R^3
    template <typename T>
    struct F : public GenericDifferentiableFunction<T>
    {
      FORWARD_TYPEDEFS ();

      explicit F () : DifferentiableFunction
		      (2, 3,
		       "vector between unit sphere and point (x,y,z)"),
		      point_ (3)
      {
	sphericalCoordinates (point_, ExpectedResult::x[0],
				      ExpectedResult::x[1]);
	point_ *= 2.;
      }

      ~F () throw ()
      {}

      void impl_compute (result_t& result, const argument_t& x) const throw ()
      {
	result.setZero ();
	double theta = x[0];
	double phi = x[1];
	sphericalCoordinates (result, theta, phi);
	result -= point_;
      }

      void impl_gradient (gradient_t& gradient, const argument_t& x,
			  size_type functionId = 0) const throw ()
      {
	double theta = x[0];
	double phi = x[1];
	switch (functionId)
	  {
	  case 0:
	    gradient[0] = -sin(theta) * cos(phi);
	    gradient[1] = -cos(theta) * sin(phi);
	    break;
	  case 1:
	    gradient[0] = cos(theta) * cos(phi);
	    gradient[1] = -sin(theta) * sin(phi);
	    break;
	  case 2:
	    gradient[0] = 0.;
	    gradient[1] = cos(phi);
	    break;
	  default:
	    abort();
	  }
      }

      static void sphericalCoordinates (result_t& res, double theta, double phi)
      {
	res (0) = cos(theta) * cos(phi);
	res (1) = sin(theta) * cos(phi);
	res (2) = sin(phi);
      }
      result_t point_;
    };
  } // namespace distanceToSphere
} // namespace roboptim


BOOST_FIXTURE_TEST_SUITE (distanceToSphere, TestSuiteConfiguration)

BOOST_AUTO_TEST_CASE_TEMPLATE (distanceToSphere_problem1, T, functionTypes_t)
{
  using namespace roboptim;
  using namespace roboptim::distanceToSphere;

  typedef Solver<SumOfC1Squares, boost::mpl::vector<> > solver_t;

  // Check tolerance
  double check_tol = 1e-2;

  // Build problem.
  boost::shared_ptr <F<T> > f (new F<T> ());
  SumOfC1Squares soq (f, "");

  typename solver_t::problem_t problem (soq);

  typename F<T>::argument_t x (2);
  x << ExpectedResult::x0[0], ExpectedResult::x0[1];
  problem.startingPoint () = x;

  BOOST_CHECK_CLOSE (soq (x)[0], ExpectedResult::fx0, 1e-6);

  // Initialize solver.
  SolverFactory<solver_t> factory (SOLVER_NAME, problem);
  solver_t& solver = factory ();

  // Add an optimization logger
  OptimizationLogger<solver_t> logger
    (solver, "/tmp/roboptim-shared-tests/" SOLVER_NAME "/distance-to-sphere");

  // Compute the minimum and retrieve the result.
  typename solver_t::result_t res = solver.minimum ();

  // Display solver information.
  std::cout << solver << std::endl;

  // Process the result
  switch (res.which ())
    {
    case solver_t::SOLVER_VALUE:
      {
	// Get the result.
	Result& result = boost::get<Result> (res);

	// Check final x.
	for (typename F<T>::size_type i = 0; i < result.x.size (); ++i)
	  BOOST_CHECK_CLOSE (result.x[i], ExpectedResult::x[i], check_tol);

	// Check final value.
	BOOST_CHECK_CLOSE (result.value[0], ExpectedResult::fx, check_tol);

	// Display the result.
	std::cout << "A solution has been found: " << std::endl
		  << result << std::endl;

	break;
      }

    case solver_t::SOLVER_VALUE_WARNINGS:
      {
	// Get the result.
	ResultWithWarnings& result = boost::get<ResultWithWarnings> (res);

	// Check final x.
	for (typename F<T>::size_type i = 0; i < result.x.size (); ++i)
	  BOOST_CHECK_CLOSE (result.x[i], ExpectedResult::x[i], check_tol);

	// Check final value.
	BOOST_CHECK_CLOSE (result.value[0], ExpectedResult::fx, check_tol);


	// Display the result.
	std::cout << "A solution has been found: " << std::endl
		  << result << std::endl;

	break;
      }

    case solver_t::SOLVER_NO_SOLUTION:
    case solver_t::SOLVER_ERROR:
      {
	std::cout << "A solution should have been found. Failing..."
		  << std::endl
		  << boost::get<SolverError> (res).what ()
		  << std::endl;
	BOOST_CHECK_EQUAL (res.which (), solver_t::SOLVER_VALUE);

	return;
      }
    }
}

BOOST_AUTO_TEST_SUITE_END ()
