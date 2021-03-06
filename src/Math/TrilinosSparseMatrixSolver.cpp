#include "TrilinosSparseMatrixSolver.h"

TrilinosSparseMatrixSolver::TrilinosSparseMatrixSolver(const Communicator &comm,
                                                       const std::string &solver,
                                                       const std::string &preconType)
    :
      comm_(comm)
{
    Tcomm_ = rcp(new TeuchosComm(comm.communicator()));

    belosParameters_ = rcp(new Teuchos::ParameterList());
    ifpackParameters_ = rcp(new Teuchos::ParameterList());

    linearProblem_ = rcp(new LinearProblem());
    solver_ = solverFactory_.create(solver, belosParameters_);
    solver_->setProblem(linearProblem_);

    preconType_ = preconType;
}

void TrilinosSparseMatrixSolver::setRank(int rank)
{
    auto map = rcp(new TpetraMap(Teuchos::OrdinalTraits<Tpetra::global_size_t>::invalid(), rank, 0, Tcomm_));

    if(map_.is_null() || !map_->isSameAs(*map)) //- Check if the map has changed
    {
        map_ = map;
        mat_ = Teuchos::null;
        x_ = rcp(new TpetraVector(map_, true));
        b_ = rcp(new TpetraVector(map_, false));
    }
}

void TrilinosSparseMatrixSolver::set(const SparseMatrixSolver::CoefficientList &eqn)
{
    using namespace Teuchos;

    bool newMatrix = mat_.is_null();

    if(newMatrix)
        mat_ = rcp(new TpetraMatrix(map_, 9, Tpetra::DynamicProfile));
    else
        mat_->resumeFill();

    Index minGlobalIndex = map_->getMinGlobalIndex();

    for(Index localRow = 0, nLocalRows = eqn.size(); localRow < nLocalRows; ++localRow)
    {
        std::vector<Scalar> vals;
        std::vector<Index> cols;

        for(const auto& entry: eqn[localRow])
        {
            cols.push_back(entry.first);
            vals.push_back(entry.second);
        }

        if(newMatrix)
            mat_->insertGlobalValues(localRow + minGlobalIndex, cols.size(), vals.data(), cols.data());
        else
            mat_->replaceGlobalValues(localRow + minGlobalIndex, cols.size(), vals.data(), cols.data());
    }

    mat_->fillComplete();

    if(newMatrix)
    {
        preconditioner_ = preconditionerFactory_.create(preconType_, rcp_implicit_cast<const TpetraMatrix>(mat_));
        preconditioner_->setParameters(*ifpackParameters_);
        preconditioner_->initialize();
        preconditioner_->compute();

        linearProblem_->setOperator(mat_);
        linearProblem_->setRightPrec(preconditioner_);
    }
}

void TrilinosSparseMatrixSolver::setRhs(const Vector &rhs)
{
    std::transform(rhs.begin(), rhs.end(), b_->getDataNonConst().begin(), [](Scalar val) { return val; });
}

Scalar TrilinosSparseMatrixSolver::solve()
{
    linearProblem_->setProblem(x_, b_);
    solver_->solve();

    return error();
}

void TrilinosSparseMatrixSolver::mapSolution(ScalarFiniteVolumeField &field)
{
    Teuchos::ArrayRCP<const Scalar> soln = x_->getData();
    for(const Cell& cell: field.grid.localActiveCells())
        field(cell) = soln[cell.index(0)];
}

void TrilinosSparseMatrixSolver::mapSolution(VectorFiniteVolumeField &field)
{
    Teuchos::ArrayRCP<const Scalar> soln = x_->getData();
    Index nActiveCells = field.grid.localActiveCells().size();

    for(const Cell& cell: field.grid.localActiveCells())
    {
        field(cell).x = soln[cell.index(0)];
        field(cell).y = soln[cell.index(0) + nActiveCells];
    }
}

void TrilinosSparseMatrixSolver::setMaxIters(int maxIters)
{
    belosParameters_->set( "Maximum Iterations", maxIters);
    solver_->setParameters(belosParameters_);
}

void TrilinosSparseMatrixSolver::setToler(Scalar toler)
{
    belosParameters_->set( "Convergence Tolerance", toler);
    solver_->setParameters(belosParameters_);
}

void TrilinosSparseMatrixSolver::setDropToler(Scalar toler)
{
    ifpackParameters_->set("fact: relax value", toler);
}

void TrilinosSparseMatrixSolver::setFillFactor(int fill)
{
    ifpackParameters_->set("fact: iluk level-of-fill", fill);
}

void TrilinosSparseMatrixSolver::setMaxPreconditionerUses(int maxPreconditionerUses)
{

}

int TrilinosSparseMatrixSolver::nIters() const
{
    return solver_->getNumIters();
}

Scalar TrilinosSparseMatrixSolver::error() const
{
    return solver_->achievedTol();
}

void TrilinosSparseMatrixSolver::printStatus(const std::string& msg) const
{
    comm_.printf("%s iterations = %d, error = %lf.\n", msg.c_str(), nIters(), error());
}
