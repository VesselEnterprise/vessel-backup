<?php

namespace App;

use Laravel\Scout\Searchable;
use Illuminate\Database\Eloquent\Model;
use Spatie\BinaryUuid\HasBinaryUuid;

class File extends Model
{
		use HasBinaryUuid;
		use Searchable;

		public function user()
		{
			return $this->belongsTo('App\User', 'user_id', 'user_id');
		}

		public function uploads()
    {
        return $this->hasMany('App\FileUpload', 'file_id', 'file_id');
    }

		public function storageProvider() {
			return $this->hasOne('App\StorageProvider', 'provider_id', 'provider_id');
		}

		public function filePath() {
			return $this->hasOne('App\FilePath', 'path_id', 'file_path_id');
		}

		public function client() {
			return $this->hasOne('App\AppClient', 'client_id', 'client_id');
		}

		protected $fillable = [
			'file_id', 'file_name', 'file_path_id', 'file_type', 'file_size', 'hash', 'uploaded', 'encrypted', 'compressed', 'last_backup'
		];

		public function getKeyName() {
			return 'file_id';
		}

		public function getKey() {
			return $this->file_id_text;
		}

		public function getHashAttribute($value) {
			return bin2hex($value);
		}

		function toSearchableArray() {
			return [
				'file_id' => $this->file_id_text,
				'file_name' => $this->file_name,
				'file_type' => $this->file_type
			];
		}

		public $primaryKey = 'file_id';
		public $incrementing = false;
		protected $table = 'file';
		protected $uuids = ['user_id','file_path_id','provider_id','client_id'];
		protected $dates = ['created_at','updated_at','last_backup'];

}
