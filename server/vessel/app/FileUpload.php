<?php

namespace App;

use Illuminate\Database\Eloquent\Model;
use App\File;
use Spatie\BinaryUuid\HasBinaryUuid;

class FileUpload extends File
{
	use HasBinaryUuid;

	public function file() {
		return $this->belongsTo('App\File', 'file_id', 'file_id');
	}

	public function parts() {
		return $this->hasMany('App\FileUploadPart', 'upload_id', 'upload_id');
	}

	public function getHashAttribute($value) {
		return bin2hex($value);
	}

	public function getKeyName() {
		return 'upload_id';
	}

	public $primaryKey = 'upload_id';
	public $incrementing = false;
	protected $table = 'file_upload';
	protected $uuids = ['file_id','user_id'];
	protected $dates = ['created_at','updated_at'];

}
