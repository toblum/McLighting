var gulp = require('gulp'),
    connect = require('gulp-connect');

gulp.task('html', function() {
  gulp.src('*.html')
    .pipe(gulp.dest('build'))
    .pipe(connect.reload());
});
 
gulp.task('js', function() {
  gulp.src('js/*.js')
    .pipe(gulp.dest('build/js'))
    .pipe(connect.reload());
});
 
gulp.task('connect', function() {
    connect.server({
		root: 'build',
        livereload: true
    });
});

gulp.task('watch', function() {
    gulp.watch('*.html', ['html']);
    gulp.watch('js/*.js', ['js']);
});

gulp.task('default', ['watch', 'connect']);