<?php

namespace App;

use Illuminate\Notifications\Notifiable;
use Illuminate\Foundation\Auth\User as Authenticatable;
use Laravel\Passport\HasApiTokens;
use Spatie\BinaryUuid\HasBinaryUuid;
use Illuminate\Support\Facades\DB;
use Laravel\Scout\Searchable;

class User extends Authenticatable
{
    use Notifiable;
    use HasBinaryUuid;
    use HasApiTokens;
		use Searchable;

    public function getKeyName()
    {
        return 'user_id';
    }

		public static function getLastInsertId()
		{
			return DB::getPdo()->lastInsertId();
		}

    public function roles()
    {
      return $this->belongsToMany('App\Role', 'role_user', 'user_id', 'role_id');
    }

		public function files()
		{
			return $this->hasMany('App\File', 'user_id', 'user_id');
		}

		public function clients()
		{
			return $this->belongsToMany('App\AppClient', 'app_client_user', 'user_id', 'client_id');
		}

		public function settings() {
			return $this->hasMany('App\UserSetting', 'user_id', 'user_id');
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
			return $this->hasMany('App\StorageProviderUser', 'user_id', 'user_id');
		}

		public function getKey()
    {
        return $this->user_id_text;
    }

		public function toSearchableArray() {
			return [
				'user_id' => $this->user_id_text,
				'email' => $this->email,
				'user_name' => $this->user_name,
				'first_name' => $this->first_name,
				'last_name' => $this->last_name,
				'title' => $this->title,
				'office' => $this->office
			];
		}

		protected $primaryKey = 'user_id';
		public $incrementing = false;
		protected $table = 'users';
		protected $uuids = ['user_id'];

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
        'password', 'remember_token'
    ];

		protected $dates = [
        'created_at',
        'updated_at',
        'last_login',
				'last_backup',
				'last_check_in'
    ];

		public $asYouType = true;

}
