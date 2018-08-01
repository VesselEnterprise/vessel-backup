<?php

namespace App\Http\Middleware;

use App\Deployment;
use Closure;

class VerifyDeploymentKey
{
    /**
     * Handle an incoming request.
     *
     * @param  \Illuminate\Http\Request  $request
     * @param  \Closure  $next
     * @return mixed
     */
    public function handle($request, Closure $next)
    {

				//Get the bearer token
				$token = Request::header('Authorization');

				if ( empty($request->bearerToken()) ) {
					abort(403, 'Not authorized');
				}

				$result = Deployment::where('deployment_key', $request->bearerToken() )->whereDate('expires_at', '>=', now() );

				if ( $result->count() <= 0 ) {
					abort(403, 'Not authorized');
				}

        return $next($request);
    }
}
