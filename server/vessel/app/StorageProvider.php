<?php

namespace App;

use Illuminate\Database\Eloquent\Model;
use Illuminate\Support\Facades\Crypt;
use Spatie\BinaryUuid\HasBinaryUuid;

class StorageProvider extends Model
{

		use HasBinaryUuid;

		public function getKeyName()
		{
				return 'provider_id';
		}

		function getAccessKeyAttribute($value) {

			$decrypted = null;

			try {
				$decrypted = decrypt($value);
			}
			catch (DecryptException $e) {
				//
			}

			return $decrypted;

		}

		function setAccessKeyAttribute($value) {

			$this->attributes['access_key'] = encrypt($value);

		}

		public function users()
    {
        return $this->hasMany('App\StorageProviderUser', 'provider_id', 'provider_id');
    }

		public function files() {
			return $this->hasMany('App\StorageProviderFile', 'provider_id', 'provider_id');
		}

		protected $fillable = ['provider_name'];
    protected $table = 'storage_provider';
		protected $hidden = ['access_id', 'access_key','created_at','updated_at'];
}
