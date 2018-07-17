<?php

namespace App;

use Illuminate\Database\Eloquent\Model;
use Spatie\BinaryUuid\HasBinaryUuid;

class File extends Model
{

		use HasBinaryUuid;

		public function getKeyName()
		{
				return 'file_id';
		}

		public function user()
		{
			return $this->belongsTo('App\User', 'user_id', 'user_id');
		}

		public function uploads()
    {
        return $this->hasMany('App\FileUpload', 'file_id', 'file_id');
    }

		public function storageProvider() {
			return $this->hasMany('App\StorageProviderFile', 'file_id', 'file_id');
		}

		/**

		 ** Role Authorization
		 ** Credit: https://medium.com/@ezp127/laravel-5-4-native-user-authentication-role-authorization-3dbae4049c8a

		 **/

		 public function authorizeRoles($role) {

			 if ( is_array($roles) ) {
				 return $this->hasAnyRole($roles) || abort(401,'This action is unauthorized');
			 }

			 return $this->hasRoles($roles) || abort(401, 'This action is unauthorized');
		 }

		 public function hasAnyRole($roles) {
			 return null !== $this->roles()->whereIn('name', $roles)->first();
		 }

		 public function hasRole($role) {
			 return null !== $this->roles()->where('name', $role)->first();
		 }

		protected $fillable = [
			'file_id', 'user_id', 'file_name', 'file_path_id', 'file_type', 'file_size', 'hash', 'uploaded', 'encrypted', 'compressed', 'last_backup'
		];

		protected $table = 'file';

		protected $dates = [
			'created_at',
			'updated_at',
			'last_backup'
		];
}
