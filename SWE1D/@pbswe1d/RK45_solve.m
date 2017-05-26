function [ obj ] = RK45_solve( obj )
%SOLVE Summary of this function goes here
%   Detailed explanation goes here

rk4a = [            0.0 ...
        -567301805773.0/1357537059087.0 ...
        -2404267990393.0/2016746695238.0 ...
        -3550918686646.0/2091501179385.0  ...
        -1275806237668.0/842570457699.0];
rk4b = [ 1432997174477.0/9575080441755.0 ...
         5161836677717.0/13612068292357.0 ...
         1720146321549.0/2090206949498.0  ...
         3134564353537.0/4481467310338.0  ...
         2277821191437.0/14882151754819.0];
rk4c = [             0.0  ...
         1432997174477.0/9575080441755.0 ...
         2526269341429.0/6820363962896.0 ...
         2006345519317.0/3224310063776.0 ...
         2802321613138.0/2924317926251.0];

time = 0;
ftime = obj.ftime;

resQ = zeros(obj.mesh.cell.Np, obj.mesh.K, obj.Nfield);
f_Q  = obj.f_Q;
obj.wetdry_detector( f_Q );

% xc = obj.mesh.cell_mean(obj.mesh.x);
while(time < ftime)
    dt = obj.time_interval;
    if(time + dt > ftime)
        dt = ftime - time;
    end
    for INTRK = 1:5
        %tloc = time + rk4c(INTRK)*dt;
        %obj.update_ext(tloc);
        rhsQ = rhs_term(obj, f_Q);
        resQ = rk4a(INTRK)*resQ + dt*rhsQ;
        
        f_Q = f_Q + rk4b(INTRK)*resQ;
        f_Q(:,:,1) = obj.slopelimiter.limit( f_Q(:,:,1), obj.M );
        f_Q(:,:,2) = obj.slopelimiter.limit( f_Q(:,:,2), obj.M );
        
        f_Q = obj.positive_preserve( f_Q );
        obj.wetdry_detector( f_Q ) ; % 重新判断干湿单元    
    end
    time = time + dt;
    obj.draw( f_Q ); pause(1e-10);
end

obj.f_Q = f_Q;
end

