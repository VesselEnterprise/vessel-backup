<?php

namespace App\Http\Middleware;

use App;
use Closure;

class VerifyClientToken
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

				$result = App\AppClient::where('token', $request->bearerToken() );

				if ( $result->count() <= 0 ) {
					return response()->json(['error' => 'Not authorized'], 403);
				}

        return $next($request);
    }
}
