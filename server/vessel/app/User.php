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
        'user_id', 'name', 'email', 'password',
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
