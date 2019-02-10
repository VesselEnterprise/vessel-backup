<?php

namespace App;

use App;
use Illuminate\Database\Eloquent\Model;
use Adldap\Adldap;

class LdapServer extends Model
{

		private $ldapAttributeMap = [
			'active_directory' => [
				'email' => 'mail',
				'user_name' => 'sAMAccountName',
				'first_name' => 'givenName',
				'last_name' => 'sn',
				'address' => 'street',
				'city' => 'l',
				'state' => 'st',
				'zip' => 'postalCode',
				'title' => 'title',
				'office' => 'physicalDeliveryOfficeName',
				'phone' => 'telephoneNumber',
				'mobile' => 'mobile'
			],
			'open_ldap' => [
				'email' => 'mail',
				'user_name' => 'uid',
				'first_name' => 'givenName',
				'last_name' => 'sn',
				'address' => 'street',
				'city' => 'l',
				'state' => 'st',
				'zip' => 'postalCode',
				'title' => 'title',
				'office' => 'physicalDeliveryOfficeName',
				'phone' => 'telephoneNumber',
				'mobile' => 'mobile'
			]
		];

		function getPasswordAttribute($value) {

			if ( !$value ) {
				return '';
			}

			$decrypted = null;

			try {
				$decrypted = decrypt($value);
			}
			catch (\Illuminate\Contracts\Encryption\DecryptException  $e) {
				$decrypted = $value;
			}

			return $decrypted;

		}

		function setPasswordAttribute($value) {
			$this->attributes['password'] = encrypt($value);
		}

		public function import() {

			$schema = null;
			if ( $this->type == 'active_directory') {
				$schema = \Adldap\Schemas\ActiveDirectory::class;
			}
			else if ( $this->type == 'open_ldap') {
				$schema = \Adldap\Schemas\OpenLDAP::class;
			}

			// LDAP Server Config
			$config = [

				// Mandatory Configuration Options
				'hosts'            => [$this->server],
				'base_dn'          => $this->dn,
				'username'         => $this->username,
				'password'         => $this->password,
				'port'             => $this->port,
				'schema'           => $schema, //\Adldap\Schemas\OpenLDAP::class,

				/*
				// Optional Configuration Options
				'schema'           => Adldap\Schemas\ActiveDirectory::class,
				'account_prefix'   => 'ACME-',
				'account_suffix'   => '@acme.org',
				'follow_referrals' => false,
				'use_ssl'          => false,
				'use_tls'          => false,
				'version'          => 3,
				'timeout'          => 5,

				// Custom LDAP Options
				'custom_options'   => [
					// See: http://php.net/ldap_set_option
					LDAP_OPT_X_TLS_REQUIRE_CERT => LDAP_OPT_X_TLS_HARD
				]
				*/

			];

			$attributeMap = $this->ldapAttributeMap[$this->type];

			$server = new \Adldap\Adldap();
			$server->addProvider($config, $this->name);

			try {

				$provider = $server->connect($this->name);
				$users = $provider->search()->users()->get();

				//Sync users
				foreach ( $users as $user ) {

					$email = $user->getAttribute($attributeMap['email'], 0);

					if ( !$email ) {
						echo "Skipped " . $user->getAttribute($attributeMap['user_name'], 0) . " - email does not exist\n";
						continue;
					}

					$dbUser = App\User::firstOrNew(['email' => $email]);

					foreach($attributeMap as $key => $value) {
						$attrValue = $user->getAttribute($value, 0);
						if ( $attrValue ) {
							$dbUser[$key] = $attrValue;
						}
						else if ( $key == 'first_name') {
							$cn = $user->getAttribute('cn', 0);
							if ( $cn ) {
								$dbUser->first_name = explode(" ", $cn)[0];
							}
						}
					}

					$dbUser->save();

					echo "Inserted or updated " . $email . "\n";

				}


			}
			catch (Adldap\Auth\BindException $e) {
			    echo "Failed to connect to LDAP server";
			}

		}

		protected $table = 'ldap_server';
		protected $hidden = ['password'];
}
