<?php

namespace App;

use Illuminate\Database\Eloquent\Model;

class StorageProviderUser extends StorageProvider
{
		public function provider()
    {
        return $this->belongsTo('App\StorageProvider');
    }

		public function user()
    {
        return $this->belongsTo('App\User');
    }

		protected $table = 'storage_provider_user';

}
