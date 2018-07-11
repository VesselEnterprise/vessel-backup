<?php

namespace App\Http\Middleware;

use Closure;

class AuthorizeRole
{
    /**
     * Handle an incoming request.
     *
     * @param  \Illuminate\Http\Request  $request
     * @param  \Closure  $next
     * @return mixed
     */
    public function handle($request, Closure $next, $role)
    {
				if (! $request->user()->hasRole($role)) {
            abort(403, 'This action is unauthorized');
        }

        return $next($request);
    }
}
