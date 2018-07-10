<?php

namespace App;

use Illuminate\Notifications\Notifiable;
use Illuminate\Foundation\Auth\User as Authenticatable;
use Laravel\Passport\HasApiTokens;
use Spatie\BinaryUuid\HasBinaryUuid;

class User extends Authenticatable
{
    use Notifiable;
    use HasBinaryUuid;
    use HasApiTokens;

    public function getKeyName()
    {
        return 'user_id';
    }

    public function roles()
    {
      return $this->belongsToMany('App\Role', 'role_user', 'user_id', 'role_id');
    }

		public function files()
		{
			return $this->hasMany('App\File', 'user_id', 'user_id');
		}

		//Credit for role authorization funcs: https://medium.com/@ezp127/laravel-5-4-native-user-authentication-role-authorization-3dbae4049c8a

		/**
		* @param string|array $roles
		*/
		public function authorizeRoles($roles)
		{
			if (is_array($roles)) {
				return $this->hasAnyRole($roles) || abort(401, 'This action is unauthorized.');
			}
			return $this->hasRole($roles) || abort(401, 'This action is unauthorized.');
		}

		/**
		* Check multiple roles
		* @param array $roles
		*/
		public function hasAnyRole($roles)
		{
			return null !== $this->roles()->whereIn('name', $roles)->first();
		}

		/**
		* Check one role
		* @param string $role
		*/
		public function hasRole($role)
		{
			return null !== $this->roles()->where('name', $role)->first();
		}

		public function storageProviders()
		{
			return $this->hasMany('App\StorageProvider');
		}

    /**
     * The attributes that are mass assignable.
     *
     * @var array
     */
    protected $fillable = [
        'user_id', 'first_name', 'last_name', 'email', 'password'
    ];

    /**
     * The attributes that should be hidden for arrays.
     *
     * @var array
     */
    protected $hidden = [
        'password', 'remember_token',
    ];

}
