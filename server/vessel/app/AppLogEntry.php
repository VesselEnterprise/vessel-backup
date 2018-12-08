<?php

namespace App;

use Illuminate\Database\Eloquent\Model;
use Laravel\Scout\Searchable;
use Spatie\BinaryUuid\HasBinaryUuid;

class AppLogEntry extends Model
{

		use Searchable;

		public function user()
		{
				return $this->hasOne('App\User', 'user_id', 'user_id');
		}

		public function client()
		{
				return $this->hasOne('App\AppClient', 'client_id', 'client_id');
		}

		function toSearchableArray() {
			return [
				'client_id' => !empty($this->client_id) ? HasBinaryUuid::decodeUuid($this->client_id) : '',
				'user_id' => !empty($this->user_id) ? HasBinaryUuid::decodeUuid($this->user_id) : '',
				'message' => $this->message,
				'type' => $this->type,
				'code' => $this->code
			];
		}

    protected $table = 'app_log';
		protected $uuids = ['user_id', 'client_id'];
		protected $dates = ['logged_at','created_at', 'updated_at'];
}
