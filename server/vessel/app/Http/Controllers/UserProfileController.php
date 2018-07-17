<?php

namespace App\Http\Controllers;

use Auth;
use App\User;
use App\Http\Controllers\Controller;
use Illuminate\Support\Facades\Hash;
use Illuminate\Support\Facades\Validator;
use Illuminate\Http\Request;

class UserProfileController extends Controller
{

		public function __construct()
	  {
	      $this->middleware('auth');
		}

		protected function validator(array $request)
		{
				return Validator::make($request, [
						'first_name' => 'required|string|max:255',
						'last_name' => 'required|string|max:255',
						'email' => 'required|string|email|max:255|unique:users',
						'password' => 'sometimes|required|string|min:6|confirmed',
				]);
		}

		private function authorizeAction($id)
		{
			//Make sure the user is accessing their own profile, or is an admin
			if ( Auth::user()->uuid_text != $id && !Auth::user()->hasRole('admin') )
			{
				abort(401, 'This action is not authorized');
			}
		}

    /**
     * Show the profile for the given user.
     *
     * @param  int  $id
     * @return Response
     */
    public function show($id)
    {
				//Make sure the user is accessing their own profile, or is an admin
				$this->authorizeAction($id);

        return view('user.profile', ['user' => User::withUuid($id)->first()]);
    }

		public function update($id, Request $request)
		{

			//Make sure the user is accessing their own profile, or is an admin
			$this->authorizeAction($id);

			$user = User::withUuid($id)->first();
			$user->first_name = $request->input('first_name');
			$user->last_name = $request->input('last_name');
			$user->email = $request->input('email');
			$user->title = $request->input('title');
			$user->office = $request->input('office');
			$user->mobile = $request->input('mobile');
			$user->address = $request->input('address');
			$user->city = $request->input('city');
			$user->state = $request->input('state');
			$user->zip = $request->input('zip');

			//Only update the password if provided
			if ( $request->input('password') != '' )
			{
				$user->password = Hash::make($request->input('password'));
			}

			$user->save();

			//Conditionally reset password

			return view('user.profile', [
				'user' => User::withUuid($id)->first(),
				'status' => 'Profile has been updated'
			]);
		}
}

?>
