<?php

namespace App\Http\Controllers\Auth;

use App\Http\Controllers\Controller;
use Illuminate\Support\Facades\Auth;
use Illuminate\Foundation\Auth\AuthenticatesUsers;
use Illuminate\Http\Request;

class LoginController extends Controller
{
    /*
    |--------------------------------------------------------------------------
    | Login Controller
    |--------------------------------------------------------------------------
    |
    | This controller handles authenticating users for the application and
    | redirecting them to your home screen. The controller uses a trait
    | to conveniently provide its functionality to your applications.
    |
    */

    use AuthenticatesUsers;

    /**
     * Where to redirect users after login.
     *
     * @var string
     */
    protected $redirectTo = '/home';

    /**
     * Create a new controller instance.
     *
     * @return void
     */
    public function __construct()
    {
        $this->middleware('guest')->except('logout');
    }

		public function login(Request $request)
    {

				$userName = $request->input('username');
				$password = $request->input('password');

				$authenticated=false;

				//Allow login with user_name or email

        if ( Auth::attempt(['email' => $userName, 'password' => $password, 'active' => 1]) ) {
					$authenticated=true;
				}
				else if( Auth::attempt(['user_name' => $userName, 'password' => $password, 'active' => 1]) ) {
					$authenticated=true;
				}
				else {
					$authenticated=false;
				}

				if ( $authenticated ) {

						//Update the user last login
						$user = Auth::user();
						$user->last_login = now();
						$user->save();

            return redirect()->intended('home');
        }
				else {
					return view('auth.login', ['error' => 'There was an issue with your username or password']);
				}
    }

}
