#ifndef SIMPLE_H
#define SIMPLE_H

#include "Solver.h"
#include "Input.h"
#include "FiniteVolumeGrid2D.h"
#include "ScalarFiniteVolumeField.h"
#include "VectorFiniteVolumeField.h"
#include "Equation.h"

class Simple : public Solver
{
public:

    Simple(const FiniteVolumeGrid2D& grid, const Input& input);

    virtual Scalar solve(Scalar timeStep);

    VectorFiniteVolumeField &u, &h;
    ScalarFiniteVolumeField &p, &pCorr, &rho, &mu, &m, &d;

protected:

    Scalar solveUEqn(Scalar timeStep);
    Scalar solvePCorrEqn();

    void computeD();
    void computeH();
    void rhieChowInterpolation();
    void correctPressure();
    void correctVelocity();
    void correctMassFlow();

    Vector2D g_;

    Equation<VectorFiniteVolumeField> uEqn_;
    Equation<ScalarFiniteVolumeField> pCorrEqn_;
};

#endif
