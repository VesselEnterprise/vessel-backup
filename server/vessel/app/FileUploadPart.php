<?php

namespace App;

use Illuminate\Database\Eloquent\Model;
use App\FileUpload;
use Spatie\BinaryUuid\HasBinaryUuid;

class FileUploadPart extends FileUpload
{
		use HasBinaryUuid;

		public function upload() {
			return $this->belongsTo('App\FileUpload', 'upload_id', 'upload_id');
		}

		public function file() {
			return $this->upload()->belongsTo('App\File', 'file_id', 'file_id');
		}

		public function getHashAttribute($value) {
			return bin2hex($value);
		}

		public function getKeyName() {
			return 'part_id';
		}

		public $incrementing = false;
		public $primaryKey = 'part_id';
    protected $table = 'file_upload_part';
		protected $uuids = ['upload_id'];
		protected $dates = ['created_at','updated_at'];
}
