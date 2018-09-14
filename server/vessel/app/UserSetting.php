<?php

namespace App;

use Illuminate\Database\Eloquent\Model;
use Spatie\BinaryUuid\HasBinaryUuid;

class UserSetting extends Model
{

		use HasBinaryUuid;

		public function user() {
			return $this->belongsTo('App\User', 'user_id', 'user_id');
		}

		public function setting() {
			return $this->belongsTo('App\AppSetting', 'setting_id', 'setting_id');
		}

    protected $table = 'app_setting_user';
		protected $fillable = ['setting_id', 'user_id'];
		protected $uuids = ['user_id'];
}
