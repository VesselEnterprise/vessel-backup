<?php

namespace App;

use Laravel\Scout\Searchable;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Support\Facades\Crypt;
use Spatie\BinaryUuid\HasBinaryUuid;

class StorageProvider extends Model
{

		use HasBinaryUuid;
		use Searchable;

		function getAccessKeyAttribute($value) {

			if ( !$value ) {
				return '';
			}

			$decrypted = null;

			try {
				$decrypted = decrypt($value);
			}
			catch (\Illuminate\Contracts\Encryption\DecryptException $e) {}

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

		public function getKeyName() {
			return 'provider_id';
		}

		public function getKey()
		{
				return $this->provider_id_text;
		}

		public function toSearchableArray() {
			return [
				'provider_id' => $this->provider_id_text,
				'provider_name' => $this->provider_name,
				'server' => $this->server,
				'region' => $this->region,
				'provider_type' => $this->provider_type
			];
		}

		public $incrementing = false;
		protected $primaryKey = 'provider_id';
    protected $table = 'storage_provider';
		protected $hidden = ['access_key','created_at','updated_at'];

}
