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

				if ( empty($request->bearerToken()) ) {
					return response()->json(['error' => 'Not authorized'], 403);
				}

				$result = Deployment::where('deployment_key', $request->bearerToken() )->whereDate('expires_at', '>=', now() );

				if ( $result->count() <= 0 ) {
					return response()->json(['error' => 'Not authorized'], 403);
				}

        return $next($request);
    }
}