#include "Laplacian.h"

namespace fv
{

Equation<ScalarFiniteVolumeField> laplacian(const ScalarFiniteVolumeField& gamma, ScalarFiniteVolumeField& field)
{
    const size_t nCells = field.grid.nActiveCells();

    std::vector<Equation<ScalarFiniteVolumeField>::Triplet> entries;
    Equation<ScalarFiniteVolumeField> eqn(field);

    entries.reserve(5*nCells);

    for(const Cell& cell: field.grid.fluidCells())
    {
        size_t row = cell.globalIndex();
        Scalar centralCoeff = 0.;

        for(const InteriorLink &nb: cell.neighbours())
        {
            size_t col = nb.cell().globalIndex();
            Scalar coeff = gamma.faces()[nb.face().id()]*dot(nb.rCellVec(), nb.outwardNorm())/dot(nb.rCellVec(), nb.rCellVec());
            centralCoeff -= coeff;

            entries.push_back(Equation<ScalarFiniteVolumeField>::Triplet(row, col, coeff));
        }

        for(const BoundaryLink &bd: cell.boundaries())
        {
            Scalar coeff = gamma.faces()[bd.face().id()]*dot(bd.rFaceVec(), bd.outwardNorm())/dot(bd.rFaceVec(), bd.rFaceVec());

            switch(field.boundaryType(bd.face().id()))
            {
            case ScalarFiniteVolumeField::FIXED:
                centralCoeff -= coeff;
                eqn.boundaries()(row) -= coeff*field.faces()[bd.face().id()];
                break;

            case ScalarFiniteVolumeField::NORMAL_GRADIENT:
                eqn.boundaries()(row) -= bd.outwardNorm().mag()/coeff*field.boundaryRefValue(bd.face().id());
                break;

            case ScalarFiniteVolumeField::SYMMETRY:
                break;

            default:
                throw Exception("fv", "laplacian", "unrecognized or unspecified boundary type.");
            }
        }

        entries.push_back(Equation<ScalarFiniteVolumeField>::Triplet(row, row, centralCoeff));
    }

    eqn.matrix().assemble(entries);
    return eqn;
}

Equation<VectorFiniteVolumeField> laplacian(const ScalarFiniteVolumeField& gamma, VectorFiniteVolumeField& field)
{
    const size_t nActiveCells = field.grid.nActiveCells();

    std::vector<Equation<VectorFiniteVolumeField>::Triplet> entries;
    Equation<VectorFiniteVolumeField> eqn(field);

    entries.reserve(10*nActiveCells);

    for(const Cell& cell: field.grid.fluidCells())
    {
        size_t rowX = cell.globalIndex();
        size_t rowY = rowX + nActiveCells;

        Scalar centralCoeff = 0.;

        for(const InteriorLink &nb: cell.neighbours())
        {
            size_t colX = nb.cell().globalIndex();
            size_t colY = colX + nActiveCells;

            Scalar coeff = gamma.faces()[nb.face().id()]*dot(nb.rCellVec(), nb.outwardNorm())/dot(nb.rCellVec(), nb.rCellVec());
            centralCoeff -= coeff;

            entries.push_back(Equation<VectorFiniteVolumeField>::Triplet(rowX, colX, coeff));
            entries.push_back(Equation<VectorFiniteVolumeField>::Triplet(rowY, colY, coeff));
        }

        for(const BoundaryLink &bd: cell.boundaries())
        {
            Scalar coeff = gamma.faces()[bd.face().id()]*dot(bd.rFaceVec(), bd.outwardNorm())/dot(bd.rFaceVec(), bd.rFaceVec());
            Vector2D source, nWall;

            switch(field.boundaryType(bd.face().id()))
            {
            case ScalarFiniteVolumeField::FIXED:
                centralCoeff -= coeff;
                eqn.boundaries()(rowX) -= coeff*field.faces()[bd.face().id()].x;
                eqn.boundaries()(rowY) -= coeff*field.faces()[bd.face().id()].y;

                break;

            case ScalarFiniteVolumeField::NORMAL_GRADIENT:
                source = bd.outwardNorm().mag()/coeff*field.boundaryRefValue(bd.face().id());
                eqn.boundaries()(rowX) -= source.x;
                eqn.boundaries()(rowY) -= source.y;
                break;

            case ScalarFiniteVolumeField::SYMMETRY:
                nWall = bd.outwardNorm().unitVec();

                entries.push_back(Equation<VectorFiniteVolumeField>::Triplet(rowX, rowX, -coeff*nWall.x*nWall.x));
                entries.push_back(Equation<ScalarFiniteVolumeField>::Triplet(rowX, rowY, -coeff*nWall.y*nWall.x));

                entries.push_back(Equation<VectorFiniteVolumeField>::Triplet(rowY, rowY, -coeff*nWall.y*nWall.y));
                entries.push_back(Equation<ScalarFiniteVolumeField>::Triplet(rowY, rowX, -coeff*nWall.x*nWall.y));
                break;

            default:
                throw Exception("fv", "laplacian", "unrecognized or unspecified boundary type.");
            }
        }

        entries.push_back(Equation<VectorFiniteVolumeField>::Triplet(rowX, rowX, centralCoeff));
        entries.push_back(Equation<VectorFiniteVolumeField>::Triplet(rowY, rowY, centralCoeff));
    }

    eqn.matrix().assemble(entries);
    return eqn;
}

}
