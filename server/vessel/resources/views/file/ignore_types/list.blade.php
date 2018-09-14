@extends('layouts.app')

@section ('scripts')
	<script src="{{ asset('js/tablesort.min.js') }}"></script>
	<script>
		$(document).ready( function() {

			$('.message .close')
				.on('click', function() {
					$(this)
						.closest('.message')
						.transition('fade');
				});

			$('.menu .item').tab();

			var list_form = $('#list_form');

			$('table').tablesort();

			$('#delete_selected').click(function() {
				$('input[name="_method"]').val('POST');
				list_form.attr('action', '{{ route('file_ignore_type.destroyMultiple') }}');
				list_form.submit();
			});

			$('#add_ignore_type').click(function(e) {
				e.preventDefault();
				window.location.href= '{{ route('file_ignore_type.create') }}';
			});

		});
	</script>
@endsection

@section('content')

	<div class="ui very padded container">

		@if(isset($success))
		<div class="ui positive message">
			<i class="close icon"></i>
			<div class="header">
				{{ $success }}
			</div>
		</div>
		@endif

		@if(isset($error))
		<div class="ui negative message">
			<i class="close icon"></i>
			<div class="header">
				{{ $error }}
			</div>
		</div>
		@endif

		<h2 class="ui header">
			Ignored Filetypes
			<div class="sub header">View and Manage Ignored File Types</div>
		</h2>
	</div>

	<form id="list_form" action="" method="POST">
		@csrf
		<input type="hidden" name="_method" value="post" />

		<div class="ui very padded container">
			<table class="ui selectable sortable stackable padded striped table">
				<thead>
					<tr>
						<th class="no-sort" colspan="8">
			      	{{ $ignoreTypes->links() }}
			    	</th>
				  </tr>
					<tr>
						<th class="no-sort"></th>
						<th>File Extension</th>
					</tr>
				</thead>
				<tbody>
					@foreach ($ignoreTypes as $type)
						<tr>
							<td>
								<div class="ui fitted checkbox">
				          <input name="selectedIds[]" type="checkbox" value="{{ $type->id }}"><label></label>
				        </div>
							</td>
							<td><a href="{{ route('file_ignore_type.show', $type->id) }}">{{ $type->file_type }}</td>
						</tr>
					@endforeach
				</tbody>
				<tfoot>
			    <tr>
						<th colspan="8">
							<div class="ui">
								<button id="add_ignore_type" class="ui primary button">
									<i class="icon plus"></i>
								  Add
								</button>
								<button id="delete_selected" class="red ui button">
									<i class="icon delete"></i>
								  Delete
								</button>
							</div>
				      {{ $ignoreTypes->links() }}
			    	</th>
				  </tr>
				</tfoot>
			</table>
		</div>

	</form>

@endsection
