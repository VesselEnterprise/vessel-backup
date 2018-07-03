<?php

namespace App;

use Illuminate\Database\Eloquent\Model;
use App\FileUpload;
use Spatie\BinaryUuid\HasBinaryUuid;

class FileUploadPart extends FileUpload
{

		use HasBinaryUuid;

		public function getKeyName() {
			return 'part_id';
		}

		public function upload() {
			return $this->belongsTo('App\FileUpload', 'upload_id', 'upload_id');
		}

		public function file() {
			return $this->upload()->belongsTo('App\File', 'file_id', 'file_id');
		}

		protected $fillable = ['part_number', 'total_bytes'];
    protected $table = 'file_upload_part';
}
