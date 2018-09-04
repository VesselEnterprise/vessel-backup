<?php

namespace App;

use Illuminate\Database\Eloquent\Model;
use Spatie\BinaryUuid\HasBinaryUuid;

class FilePath extends Model
{
		use HasBinaryUuid;

		public function getHashAttribute($value) {
			return bin2hex($value);
		}

		public function getKeyName() {
			return 'path_id';
		}

		public function files() {
			return $this->hasMany('App\File', 'file_path_id', 'path_id');
		}

		public $incrementing = false;
		public $primaryKey = 'path_id';
    protected $table = 'file_path';
		protected $uuids = ['user_id'];

}
