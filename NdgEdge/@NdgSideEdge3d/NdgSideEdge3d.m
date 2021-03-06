classdef NdgSideEdge3d < handle
    properties ( SetAccess = protected )
        %> mesh obj
        mesh
        %> num of face nodes
        Nfp
        %> num of edges
        Ne
        %> mass matrix of edge
        M
        %> vertex index on each edge
        FToV
        %> local and adjacent cell index
        FToE
        %> local face index of local and adjacent cell
        FToF
        %> face to mesh index
        FToM
        %> interp node index of 1st ele on each edge
        FToN1
        %> interp node index of 2nd ele on each edge
        FToN2
        %> outward normal vector
        nx, ny, nz
        %> determination of edge Jacabian
        Js
    end

    methods ( Access = public )
        function obj = NdgSideEdge3d( meshUnion3d, meshId )
            mesh = meshUnion3d( meshId );

            [ obj.Nfp, obj.M ] = assembleMassMatrix( obj, mesh.cell.N, mesh.cell.Nz );
            obj.mesh = mesh;
            obj = assembleEdgeConnect( obj, mesh, mesh.mesh2d.InnerEdge );
            obj = assembleNodeProject( obj, mesh );
        end

        %> access boundary values at edges
        [ fM, fP ] = matEvaluateSurfValue( obj, fphys );
        %> evaluate strong-form surface term rhs
        [ frhs ] = matEvaluateStrongFromEdgeRHS( obj, fluxM, fluxP, fluxS )
    end

    methods ( Access = private )
        obj = assembleEdgeConnect( obj, mesh, edge );
        obj = assembleNodeProject( obj, mesh );
        [ Nfp, M ] = assembleMassMatrix( obj, N, Nz );
    end
end