<?php

namespace App\Http\Middleware;

use App;
use Carbon\Carbon;
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

				$result = App\Deployment::where([
					['deployment_key', '=', $request->bearerToken()],
					['expires_at', '>=', Carbon::now()->toDateTimeString()]
				])->orWhere([
					['deployment_key', '=', $request->bearerToken()],
					['never_expires', '=', 1]
				])->first();

				if ( !$result ) {
					return response()->json(['error' => 'Not authorized'], 403);
				}

        return $next($request);
    }
}
