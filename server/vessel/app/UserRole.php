<?php

namespace App;

use Illuminate\Database\Eloquent\Model;
use Spatie\BinaryUuid\HasBinaryUuid;

class UserRole extends Model
{

		public function user() {
			return $this->belongsTo('App\User', 'user_id', 'user_id');
		}

		public function role() {
			return $this->belongsTo('App\Role', 'role_id', 'id');
		}

    protected $table = 'role_user';
		protected $fillable = ['role_id', 'user_id'];
}
