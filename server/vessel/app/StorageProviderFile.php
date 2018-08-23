<?php

namespace App;

use Illuminate\Database\Eloquent\Model;

class StorageProviderFile extends StorageProvider
{

		public function provider()
		{
			return $this->belongsTo('App\StorageProvider', 'provider_id', 'provider_id');
		}

		public function file()
		{
			return $this->belongsTo('App\File', 'file_id', 'file_id');
		}

    protected $table = 'storage_provider_file';
		protected $fillable = ['provider_id', 'file_id'];
}
