#include "Piso.h"

Piso::Piso(const FiniteVolumeGrid2D &grid, const Input &input)
    :
      Simple(grid, input)
{
    nPCorrections_ = input.caseInput().get<int>("Solver.numPressureCorrections", 1);
}

Scalar Piso::solve(Scalar timeStep)
{
    u.save();

    //if(!(timeDependent_ == Solver::OFF))
        //timeStep = std::numeric_limits<Scalar>::infinity();

    for(size_t i = 0; i < nInnerIterations_; ++i)
    {
        solveUEqn(timeStep);

        for(size_t j = 0; j < nPCorrections_; ++j)
        {
            solvePCorrEqn();
            correctPressure();
            correctVelocity();
        }
    }

    printf("time step = %lf, max Co = %lf\n", timeStep, courantNumber(timeStep));

    return 0;
}